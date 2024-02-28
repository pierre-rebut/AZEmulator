//
// Created by pierr on 17/01/2024.
//

#include <cstring>
#include <cmath>

#include "Vera.h"

namespace Astra::CPU::Lib::Monitors {
    static int calc_layer_eff_x(const struct video_layer_properties* props, const int x) {
        return (x + props->hscroll) & (props->layerw_max);
    }

    static int calc_layer_eff_y(const struct video_layer_properties* props, const int y) {
        return (y + props->vscroll) & (props->layerh_max);
    }

    static DWORD calc_layer_map_addr_base2(const struct video_layer_properties* props, const int eff_x, const int eff_y) {
        // Slightly faster on some platforms because we know that tilew and tileh are powers of 2.
        return props->map_base + ((((eff_y >> props->tileh_log2) << props->mapw_log2) + (eff_x >> props->tilew_log2)) << 1);
    }

    void Vera::refresh_layer_properties(const BYTE layer) {
        struct video_layer_properties* props = &layer_properties[layer];

        WORD prev_layerw_max = props->layerw_max;
        WORD prev_hscroll = props->hscroll;

        props->color_depth = reg_layer[layer][0] & 0x3;
        props->map_base = reg_layer[layer][1] << 9;
        props->tile_base = (reg_layer[layer][2] & 0xFC) << 9;
        props->bitmap_mode = (reg_layer[layer][0] & 0x4) != 0;
        props->text_mode = (props->color_depth == 0) && !props->bitmap_mode;
        props->text_mode_256c = (reg_layer[layer][0] & 8) != 0;
        props->tile_mode = !props->bitmap_mode && !props->text_mode;

        if (!props->bitmap_mode) {
            props->hscroll = reg_layer[layer][3] | (reg_layer[layer][4] & 0xf) << 8;
            props->vscroll = reg_layer[layer][5] | (reg_layer[layer][6] & 0xf) << 8;
        } else {
            props->hscroll = 0;
            props->vscroll = 0;
        }

        WORD mapw = 0;
        WORD maph = 0;
        props->tilew = 0;
        props->tileh = 0;

        if (props->tile_mode || props->text_mode) {
            props->mapw_log2 = 5 + ((reg_layer[layer][0] >> 4) & 3);
            props->maph_log2 = 5 + ((reg_layer[layer][0] >> 6) & 3);
            mapw = 1 << props->mapw_log2;
            maph = 1 << props->maph_log2;

            // Scale the tiles or text characters according to TILEW and TILEH.
            props->tilew_log2 = 3 + (reg_layer[layer][2] & 1);
            props->tileh_log2 = 3 + ((reg_layer[layer][2] >> 1) & 1);
            props->tilew = 1 << props->tilew_log2;
            props->tileh = 1 << props->tileh_log2;
        } else if (props->bitmap_mode) {
            // bitmap mode is basically tiled mode with a single huge tile
            props->tilew = (reg_layer[layer][2] & 1) ? 640 : 320;
            props->tileh = SCREEN_HEIGHT;
        }

        // We know mapw, maph, tilew, and tileh are powers of two in all cases except bitmap modes, and any products of that set will be powers of two,
        // so there's no need to modulo against them if we have bitmasks we can bitwise-and against.

        props->mapw_max = mapw - 1;
        props->maph_max = maph - 1;
        props->tilew_max = props->tilew - 1;
        props->tileh_max = props->tileh - 1;
        props->layerw_max = (mapw * props->tilew) - 1;
        props->layerh_max = (maph * props->tileh) - 1;

        // Find min/max eff_x for bulk reading in tile data during draw.
        if (prev_layerw_max != props->layerw_max || prev_hscroll != props->hscroll) {
            int min_eff_x = INT_MAX;
            int max_eff_x = INT_MIN;
            for (int x = 0; x < SCREEN_WIDTH; ++x) {
                int eff_x = calc_layer_eff_x(props, x);
                if (eff_x < min_eff_x) {
                    min_eff_x = eff_x;
                }
                if (eff_x > max_eff_x) {
                    max_eff_x = eff_x;
                }
            }
            props->min_eff_x = min_eff_x;
            props->max_eff_x = max_eff_x;
        }

        props->bits_per_pixel = 1 << props->color_depth;
        props->tile_size_log2 = props->tilew_log2 + props->tileh_log2 + props->color_depth - 3;

        props->first_color_pos = 8 - props->bits_per_pixel;
        props->color_mask = (1 << props->bits_per_pixel) - 1;
        props->color_fields_max = (8 >> props->color_depth) - 1;
    }

    void Vera::refresh_sprite_properties(const WORD sprite) {
        struct video_sprite_properties* props = &sprite_properties[sprite];

        props->sprite_zdepth = (sprite_data[sprite][6] >> 2) & 3;
        props->sprite_collision_mask = sprite_data[sprite][6] & 0xf0;

        props->sprite_x = sprite_data[sprite][2] | (sprite_data[sprite][3] & 3) << 8;
        props->sprite_y = sprite_data[sprite][4] | (sprite_data[sprite][5] & 3) << 8;
        props->sprite_width_log2 = (((sprite_data[sprite][7] >> 4) & 3) + 3);
        props->sprite_height_log2 = ((sprite_data[sprite][7] >> 6) + 3);
        props->sprite_width = 1 << props->sprite_width_log2;
        props->sprite_height = 1 << props->sprite_height_log2;

        // fix up negative coordinates
        if (props->sprite_x >= 0x400 - props->sprite_width) {
            props->sprite_x -= 0x400;
        }
        if (props->sprite_y >= 0x400 - props->sprite_height) {
            props->sprite_y -= 0x400;
        }

        props->hflip = sprite_data[sprite][6] & 1;
        props->vflip = (sprite_data[sprite][6] >> 1) & 1;

        props->color_mode = (sprite_data[sprite][1] >> 7) & 1;
        props->sprite_address = sprite_data[sprite][0] << 5 | (sprite_data[sprite][1] & 0xf) << 13;

        props->palette_offset = (sprite_data[sprite][7] & 0x0f) << 4;
    }

    void Vera::refresh_palette() {
        const BYTE out_mode = reg_composer[0] & 3;
        const bool chroma_disable = ((reg_composer[0] & 0x07) == 6);
        for (int i = 0; i < 256; ++i) {
            BYTE r;
            BYTE g;
            BYTE b;
            if (out_mode == 0) {
                // video generation off
                // -> show blue screen
                r = 0;
                g = 0;
                b = 255;
            } else {
                WORD entry = palette[i * 2] | palette[i * 2 + 1] << 8;
                r = ((entry >> 8) & 0xf) << 4 | ((entry >> 8) & 0xf);
                g = ((entry >> 4) & 0xf) << 4 | ((entry >> 4) & 0xf);
                b = (entry & 0xf) << 4 | (entry & 0xf);
                if (chroma_disable) {
                    r = g = b = (r + b + g) / 3;
                }
            }

            video_palette.entries[i] = (DWORD) (r << 16) | ((DWORD) g << 8) | ((DWORD) b);
        }
        video_palette.dirty = false;
    }

    static void expand_4bpp_data(BYTE* dst, const BYTE* src, int dst_size) {
        while (dst_size >= 2) {
            *dst = (*src) >> 4;
            ++dst;
            *dst = (*src) & 0xf;
            ++dst;

            ++src;
            dst_size -= 2;
        }
    }

    void Vera::render_sprite_line(const WORD y) {
        std::memset(sprite_line_col, 0, SCREEN_WIDTH);
        std::memset(sprite_line_z, 0, SCREEN_WIDTH);
        std::memset(sprite_line_mask, 0, SCREEN_WIDTH);

        WORD sprite_budget = 800 + 1;
        for (int i = 0; i < NUM_SPRITES; i++) {
            // one clock per lookup
            sprite_budget--;
            if (sprite_budget == 0) break;
            const struct video_sprite_properties* props = &sprite_properties[i];

            if (props->sprite_zdepth == 0) {
                continue;
            }

            // check whether this line falls within the sprite
            if (y < props->sprite_y || y >= props->sprite_y + props->sprite_height) {
                continue;
            }

            const WORD eff_sy = props->vflip ? ((props->sprite_height - 1) - (y - props->sprite_y)) : (y - props->sprite_y);

            int16_t eff_sx = (props->hflip ? (props->sprite_width - 1) : 0);
            const int16_t eff_sx_incr = props->hflip ? -1 : 1;

            const BYTE* bitmap_data = video_ram + props->sprite_address + (eff_sy << (props->sprite_width_log2 - (1 - props->color_mode)));

            BYTE unpacked_sprite_line[64];
            const WORD width = (props->sprite_width < 64 ? props->sprite_width : 64);
            if (props->color_mode == 0) {
                // 4bpp
                expand_4bpp_data(unpacked_sprite_line, bitmap_data, width);
            } else {
                // 8bpp
                memcpy(unpacked_sprite_line, bitmap_data, width);
            }

            for (WORD sx = 0; sx < props->sprite_width; ++sx) {
                const WORD line_x = props->sprite_x + sx;
                if (line_x >= SCREEN_WIDTH) {
                    eff_sx += eff_sx_incr;
                    continue;
                }

                // one clock per fetched 32 bits
                if (!(sx & 3)) {
                    sprite_budget--;
                    if (sprite_budget == 0) break;
                }

                // one clock per rendered pixel
                sprite_budget--;
                if (sprite_budget == 0) break;

                const BYTE col_index = unpacked_sprite_line[eff_sx];
                eff_sx += eff_sx_incr;

                // palette offset
                if (col_index > 0) {
                    sprite_line_collisions |= sprite_line_mask[line_x] & props->sprite_collision_mask;
                    sprite_line_mask[line_x] |= props->sprite_collision_mask;

                    if (props->sprite_zdepth > sprite_line_z[line_x]) {
                        sprite_line_col[line_x] = col_index + props->palette_offset;
                        sprite_line_z[line_x] = props->sprite_zdepth;
                    }
                }
            }
        }
    }

    void Vera::render_layer_line_text(BYTE layer, WORD y) {
        const struct video_layer_properties* props = &prev_layer_properties[1][layer];
        const struct video_layer_properties* props0 = &prev_layer_properties[0][layer];

        const BYTE max_pixels_per_byte = (8 >> props->color_depth) - 1;
        const int eff_y = calc_layer_eff_y(props0, y);
        const int yy = eff_y & props->tileh_max;

        // additional bytes to reach the correct line of the tile
        const DWORD y_add = (yy << props->tilew_log2) >> 3;

        const DWORD map_addr_begin = calc_layer_map_addr_base2(props, props->min_eff_x, eff_y);
        const DWORD map_addr_end = calc_layer_map_addr_base2(props, props->max_eff_x, eff_y);
        const int size = (map_addr_end - map_addr_begin) + 2;

        BYTE tile_bytes[512]; // max 256 tiles, 2 bytes each.
        video_space_read_range(tile_bytes, map_addr_begin, size);

        DWORD tile_start;

        BYTE fg_color;
        BYTE bg_color;
        BYTE s;
        BYTE color_shift;

        {
            const int eff_x = calc_layer_eff_x(props, 0);
            const int xx = eff_x & props->tilew_max;

            // extract all information from the map
            const DWORD map_addr = calc_layer_map_addr_base2(props, eff_x, eff_y) - map_addr_begin;

            const BYTE tile_index = tile_bytes[map_addr];
            const BYTE byte1 = tile_bytes[map_addr + 1];

            if (!props->text_mode_256c) {
                fg_color = byte1 & 15;
                bg_color = byte1 >> 4;
            } else {
                fg_color = byte1;
                bg_color = 0;
            }

            // offset within tilemap of the current tile
            tile_start = tile_index << props->tile_size_log2;

            // additional bytes to reach the correct column of the tile
            const WORD x_add = xx >> 3;
            const DWORD tile_offset = tile_start + y_add + x_add;

            s = video_space_read(props->tile_base + tile_offset);
            color_shift = max_pixels_per_byte - (xx & 0x7);
        }

        // Render tile line.
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            // Scrolling
            const int eff_x = calc_layer_eff_x(props, x);
            const int xx = eff_x & props->tilew_max;

            if ((eff_x & 0x7) == 0) {
                if ((eff_x & props->tilew_max) == 0) {
                    // extract all information from the map
                    const DWORD map_addr = calc_layer_map_addr_base2(props, eff_x, eff_y) - map_addr_begin;

                    const BYTE tile_index = tile_bytes[map_addr];
                    const BYTE byte1 = tile_bytes[map_addr + 1];

                    if (!props->text_mode_256c) {
                        fg_color = byte1 & 15;
                        bg_color = byte1 >> 4;
                    } else {
                        fg_color = byte1;
                        bg_color = 0;
                    }

                    // offset within tilemap of the current tile
                    tile_start = tile_index << props->tile_size_log2;
                }

                // additional bytes to reach the correct column of the tile
                const WORD x_add = xx >> 3;
                const DWORD tile_offset = tile_start + y_add + x_add;

                s = video_space_read(props->tile_base + tile_offset);
                color_shift = max_pixels_per_byte;
            }

            // convert tile byte to indexed color
            const BYTE col_index = (s >> color_shift) & 1;
            --color_shift;
            layer_line[layer][x] = col_index ? fg_color : bg_color;
        }
    }

    void Vera::render_layer_line_tile(BYTE layer, WORD y) {
        const struct video_layer_properties* props = &prev_layer_properties[1][layer];
        const struct video_layer_properties* props0 = &prev_layer_properties[0][layer];

        const BYTE max_pixels_per_byte = (8 >> props->color_depth) - 1;
        const int eff_y = calc_layer_eff_y(props0, y);
        const BYTE yy = eff_y & props->tileh_max;
        const BYTE yy_flip = yy ^ props->tileh_max;
        const DWORD y_add = (yy << (props->tilew_log2 + props->color_depth - 3));
        const DWORD y_add_flip = (yy_flip << (props->tilew_log2 + props->color_depth - 3));

        const DWORD map_addr_begin = calc_layer_map_addr_base2(props, props->min_eff_x, eff_y);
        const DWORD map_addr_end = calc_layer_map_addr_base2(props, props->max_eff_x, eff_y);
        const int size = (map_addr_end - map_addr_begin) + 2;

        BYTE tile_bytes[512]; // max 256 tiles, 2 bytes each.
        video_space_read_range(tile_bytes, map_addr_begin, size);

        BYTE palette_offset;
        bool vflip;
        bool hflip;
        DWORD tile_start;
        BYTE s;
        BYTE color_shift;
        int8_t color_shift_incr;

        {
            const int eff_x = calc_layer_eff_x(props, 0);

            // extract all information from the map
            const DWORD map_addr = calc_layer_map_addr_base2(props, eff_x, eff_y) - map_addr_begin;

            const BYTE byte0 = tile_bytes[map_addr];
            const BYTE byte1 = tile_bytes[map_addr + 1];

            // Tile Flipping
            vflip = (byte1 >> 3) & 1;
            hflip = (byte1 >> 2) & 1;

            palette_offset = byte1 & 0xf0;

            // offset within tilemap of the current tile
            const WORD tile_index = byte0 | ((byte1 & 3) << 8);
            tile_start = tile_index << props->tile_size_log2;

            color_shift_incr = hflip ? props->bits_per_pixel : -props->bits_per_pixel;

            int xx = eff_x & props->tilew_max;
            if (hflip) {
                xx = xx ^ (props->tilew_max);
                color_shift = 0;
            } else {
                color_shift = props->first_color_pos;
            }

            // additional bytes to reach the correct column of the tile
            WORD x_add = (xx << props->color_depth) >> 3;
            DWORD tile_offset = tile_start + (vflip ? y_add_flip : y_add) + x_add;

            s = video_space_read(props->tile_base + tile_offset);
        }


        // Render tile line.
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            const int eff_x = calc_layer_eff_x(props, x);

            if ((eff_x & max_pixels_per_byte) == 0) {
                if ((eff_x & props->tilew_max) == 0) {
                    // extract all information from the map
                    const DWORD map_addr = calc_layer_map_addr_base2(props, eff_x, eff_y) - map_addr_begin;

                    const BYTE byte0 = tile_bytes[map_addr];
                    const BYTE byte1 = tile_bytes[map_addr + 1];

                    // Tile Flipping
                    vflip = (byte1 >> 3) & 1;
                    hflip = (byte1 >> 2) & 1;

                    palette_offset = byte1 & 0xf0;

                    // offset within tilemap of the current tile
                    const WORD tile_index = byte0 | ((byte1 & 3) << 8);
                    tile_start = tile_index << props->tile_size_log2;

                    color_shift_incr = hflip ? props->bits_per_pixel : -props->bits_per_pixel;
                }

                int xx = eff_x & props->tilew_max;
                if (hflip) {
                    xx = xx ^ (props->tilew_max);
                    color_shift = 0;
                } else {
                    color_shift = props->first_color_pos;
                }

                // additional bytes to reach the correct column of the tile
                const WORD x_add = (xx << props->color_depth) >> 3;
                const DWORD tile_offset = tile_start + (vflip ? y_add_flip : y_add) + x_add;

                s = video_space_read(props->tile_base + tile_offset);
            }

            // convert tile byte to indexed color
            BYTE col_index = (s >> color_shift) & props->color_mask;
            color_shift += color_shift_incr;

            // Apply Palette Offset
            if (col_index > 0 && col_index < 16) {
                col_index += palette_offset;
                if (props->text_mode_256c) {
                    col_index |= 0x80;
                }
            }
            layer_line[layer][x] = col_index;
        }
    }

    void Vera::render_layer_line_bitmap(BYTE layer, WORD y) {
        const struct video_layer_properties* props = &prev_layer_properties[1][layer];
//	const struct video_layer_properties *props0 = &prev_layer_properties[0][layer];

        int yy = y % props->tileh;
        // additional bytes to reach the correct line of the tile
        DWORD y_add = (yy * props->tilew * props->bits_per_pixel) >> 3;

        // Render tile line.
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int xx = x % props->tilew;

            // extract all information from the map
            BYTE palette_offset = reg_layer[layer][4] & 0xf;

            // additional bytes to reach the correct column of the tile
            WORD x_add = (xx * props->bits_per_pixel) >> 3;
            DWORD tile_offset = y_add + x_add;
            BYTE s = video_space_read(props->tile_base + tile_offset);

            // convert tile byte to indexed color
            BYTE col_index = (s >> (props->first_color_pos - ((xx & props->color_fields_max) << props->color_depth))) & props->color_mask;

            // Apply Palette Offset
            if (col_index > 0 && col_index < 16) {
                col_index += palette_offset << 4;
                if (props->text_mode_256c) {
                    col_index |= 0x80;
                }
            }
            layer_line[layer][x] = col_index;
        }
    }

    static BYTE calculate_line_col_index(BYTE spr_zindex, BYTE spr_col_index, BYTE l1_col_index, BYTE l2_col_index) {
        BYTE col_index = 0;
        switch (spr_zindex) {
            case 3:
                col_index = spr_col_index ? spr_col_index : (l2_col_index ? l2_col_index : l1_col_index);
                break;
            case 2:
                col_index = l2_col_index ? l2_col_index : (spr_col_index ? spr_col_index : l1_col_index);
                break;
            case 1:
                col_index = l2_col_index ? l2_col_index : (l1_col_index ? l1_col_index : spr_col_index);
                break;
            case 0:
                col_index = l2_col_index ? l2_col_index : l1_col_index;
                break;
        }
        return col_index;
    }

    void Vera::render_line(WORD y, float scan_pos_x) {
        static WORD y_prev;
        static WORD s_pos_x_p;
        static DWORD eff_y_fp; // 16.16 fixed point
        static DWORD eff_x_fp; // 16.16 fixed point

        static BYTE col_line[SCREEN_WIDTH];

        BYTE dc_video = reg_composer[0];
        WORD vstart = reg_composer[6] << 1;

        if (y != y_prev) {
            y_prev = y;
            s_pos_x_p = 0;

            // Copy the composer array to 2-line history buffer
            // so that the raster effects that happen on a delay take effect
            // at exactly the right time

            // This simulates different effects happening at render,
            // render but delayed until the next line, or applied mid-line
            // at scan-out

            memcpy(prev_reg_composer[1], prev_reg_composer[0], sizeof(*reg_composer) * COMPOSER_SLOTS);
            memcpy(prev_reg_composer[0], reg_composer, sizeof(*reg_composer) * COMPOSER_SLOTS);

            // Same with the layer properties

            memcpy(prev_layer_properties[1], prev_layer_properties[0], sizeof(*layer_properties) * NUM_LAYERS);
            memcpy(prev_layer_properties[0], layer_properties, sizeof(*layer_properties) * NUM_LAYERS);

            if ((dc_video & 3) > 1) { // 480i or 240p
                if ((y >> 1) == 0) {
                    eff_y_fp = y * (prev_reg_composer[1][2] << 9);
                } else if ((y & 0xfffe) > vstart) {
                    eff_y_fp += (prev_reg_composer[1][2] << 10);
                }
            } else {
                if (y == 0) {
                    eff_y_fp = 0;
                } else if (y > vstart) {
                    eff_y_fp += (prev_reg_composer[1][2] << 9);
                }
            }
        }

        if ((dc_video & 8) && (dc_video & 3) > 1) { // progressive NTSC/RGB mode
            y &= 0xfffe;
        }

        // refresh palette for next entry
        if (video_palette.dirty) {
            refresh_palette();
        }

        if (y >= SCREEN_HEIGHT) {
            return;
        }

        WORD s_pos_x = std::round(scan_pos_x);
        if (s_pos_x > SCREEN_WIDTH) {
            s_pos_x = SCREEN_WIDTH;
        }

        if (s_pos_x_p == 0) {
            eff_x_fp = 0;
        }

        BYTE out_mode = reg_composer[0] & 3;

        BYTE border_color = reg_composer[3];
        WORD hstart = reg_composer[4] << 2;
        WORD hstop = reg_composer[5] << 2;
        WORD vstop = reg_composer[7] << 1;

        WORD eff_y = (eff_y_fp >> 16);

        layer_line_enable[0] = dc_video & 0x10;
        layer_line_enable[1] = dc_video & 0x20;
        sprite_line_enable = dc_video & 0x40;

        // clear layer_line if layer gets disabled
        for (BYTE layer = 0; layer < 2; layer++) {
            if (!layer_line_enable[layer] && old_layer_line_enable[layer]) {
                for (WORD i = s_pos_x_p; i < SCREEN_WIDTH; i++) {
                    layer_line[layer][i] = 0;
                }
            }
            if (s_pos_x_p == 0)
                old_layer_line_enable[layer] = layer_line_enable[layer];
        }

        // clear sprite_line if sprites get disabled
        if (!sprite_line_enable && old_sprite_line_enable) {
            for (WORD i = s_pos_x_p; i < SCREEN_WIDTH; i++) {
                sprite_line_col[i] = 0;
                sprite_line_z[i] = 0;
                sprite_line_mask[i] = 0;
            }
        }

        if (s_pos_x_p == 0)
            old_sprite_line_enable = sprite_line_enable;


        if (sprite_line_enable) {
            render_sprite_line(eff_y);
        }

        /*if (warp_mode && (frame_count & 63)) {
            // sprites were needed for the collision IRQ, but we can skip
            // everything else if we're in warp mode, most of the time
            return;
        }*/

        if (layer_line_enable[0]) {
            if (prev_layer_properties[1][0].text_mode) {
                render_layer_line_text(0, eff_y);
            } else if (prev_layer_properties[1][0].bitmap_mode) {
                render_layer_line_bitmap(0, eff_y);
            } else {
                render_layer_line_tile(0, eff_y);
            }
        }
        if (layer_line_enable[1]) {
            if (prev_layer_properties[1][1].text_mode) {
                render_layer_line_text(1, eff_y);
            } else if (prev_layer_properties[1][1].bitmap_mode) {
                render_layer_line_bitmap(1, eff_y);
            } else {
                render_layer_line_tile(1, eff_y);
            }
        }

        // If video output is enabled, calculate color indices for line.
        if (out_mode != 0) {
            // Add border after if required.
            if (y < vstart || y > vstop) {
                DWORD border_fill = border_color;
                border_fill = border_fill | (border_fill << 8);
                border_fill = border_fill | (border_fill << 16);
                std::memset(col_line, border_fill, SCREEN_WIDTH);
            } else {
                hstart = hstart < 640 ? hstart : 640;
                hstop = hstop < 640 ? hstop : 640;

                for (WORD x = s_pos_x_p; x < hstart && x < s_pos_x; ++x) {
                    col_line[x] = border_color;
                }

                const DWORD scale = reg_composer[1];
                for (WORD x = MAX(hstart, s_pos_x_p); x < hstop && x < s_pos_x; ++x) {
                    WORD eff_x = eff_x_fp >> 16;
                    col_line[x] = calculate_line_col_index(sprite_line_z[eff_x], sprite_line_col[eff_x], layer_line[0][eff_x], layer_line[1][eff_x]);
                    eff_x_fp += (scale << 9);
                }
                for (WORD x = hstop; x < s_pos_x; ++x) {
                    col_line[x] = border_color;
                }
            }
        }

        // Look up all color indices.
        DWORD* framebuffer4_begin = ((DWORD*) framebuffer) + (y * SCREEN_WIDTH) + s_pos_x_p;
        {
            DWORD* framebuffer4 = framebuffer4_begin;
            for (WORD x = s_pos_x_p; x < s_pos_x; x++) {
                *framebuffer4++ = video_palette.entries[col_line[x]];
            }
        }

        // NTSC overscan
        if (out_mode == 2) {
            DWORD* framebuffer4 = framebuffer4_begin;
            for (WORD x = s_pos_x_p; x < s_pos_x; x++) {
                if (x < SCREEN_WIDTH * TITLE_SAFE_X ||
                    x > SCREEN_WIDTH * (1 - TITLE_SAFE_X) ||
                    y < SCREEN_HEIGHT * TITLE_SAFE_Y ||
                    y > SCREEN_HEIGHT * (1 - TITLE_SAFE_Y)) {

                    // Divide RGB elements by 4.
                    *framebuffer4 &= 0x00fcfcfc;
                    *framebuffer4 >>= 2;
                }
                framebuffer4++;
            }
        }

        s_pos_x_p = s_pos_x;
    }

    void Vera::update_isr_and_coll(WORD y, WORD compare) {
        if (y == SCREEN_HEIGHT) {
            if (sprite_line_collisions != 0) {
                isr |= 4;
            }
            isr = (isr & 0xf) | sprite_line_collisions;
            sprite_line_collisions = 0;
            isr |= 1; // VSYNC IRQ
        }
        if (y == compare) { // LINE IRQ
            isr |= 2;
        }
    }

    DWORD Vera::get_and_inc_address(BYTE sel, bool write) {
        DWORD address = io_addr[sel];
        int16_t incr = increments[io_inc[sel]];

        if (fx_4bit_mode && fx_nibble_incr[sel] && !incr) {
            if (fx_nibble_bit[sel]) {
                if ((io_inc[sel] & 1) == 0) io_addr[sel] += 1;
                fx_nibble_bit[sel] = 0;
            } else {
                if (io_inc[sel] & 1) io_addr[sel] -= 1;
                fx_nibble_bit[sel] = 1;
            }
        }

        if (sel == 1 && fx_16bit_hop) {
            if (incr == 4) {
                if (fx_16bit_hop_align == (address & 0x3))
                    incr = 1;
                else
                    incr = 3;
            } else if (incr == 320) {
                if (fx_16bit_hop_align == (address & 0x3))
                    incr = 1;
                else
                    incr = 319;
            }
        }

        io_addr[sel] += incr;

        if (sel == 1 && fx_addr1_mode == 1) { // FX line draw mode
            fx_x_pixel_position += fx_x_pixel_increment;
            if (fx_x_pixel_position & 0x10000) {
                fx_x_pixel_position &= ~0x10000;
                if (fx_4bit_mode && fx_nibble_incr[0]) {
                    if (fx_nibble_bit[1]) {
                        if ((io_inc[0] & 1) == 0) io_addr[1] += 1;
                        fx_nibble_bit[1] = 0;
                    } else {
                        if (io_inc[0] & 1) io_addr[1] -= 1;
                        fx_nibble_bit[1] = 1;
                    }
                }
                io_addr[1] += increments[io_inc[0]];
            }
        } else if (fx_addr1_mode == 2 && write == false) { // FX polygon fill mode
            fx_x_pixel_position += fx_x_pixel_increment;
            fx_y_pixel_position += fx_y_pixel_increment;
            fx_poly_fill_length = ((int32_t) fx_y_pixel_position >> 16) - ((int32_t) fx_x_pixel_position >> 16);
            if (sel == 0 && fx_cache_byte_cycling && !fx_cache_fill) {
                fx_cache_byte_index = (fx_cache_byte_index + 1) & 3;
            }
            if (sel == 1) {
                if (fx_4bit_mode) {
                    io_addr[1] = io_addr[0] + (fx_x_pixel_position >> 17);
                    fx_nibble_bit[1] = (fx_x_pixel_position >> 16) & 1;
                } else {
                    io_addr[1] = io_addr[0] + (fx_x_pixel_position >> 16);
                }
            }
        } else if (sel == 1 && fx_addr1_mode == 3 && write == false) { // FX affine mode
            fx_x_pixel_position += fx_x_pixel_increment;
            fx_y_pixel_position += fx_y_pixel_increment;
        }
        return address;
    }
}
