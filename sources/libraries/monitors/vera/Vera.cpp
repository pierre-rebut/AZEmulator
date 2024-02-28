//
// Created by pierr on 17/01/2024.
//
#include <cstring>
#include <iostream>
#include "Vera.h"

namespace Astra::CPU::Lib::Monitors {
    const BYTE Vera::vera_version_string[] = {
            'V',
            VERA_VERSION_MAJOR,
            VERA_VERSION_MINOR,
            VERA_VERSION_PATCH
    };

    const WORD Vera::default_palette[] = {
            0x000, 0xfff, 0x800, 0xafe, 0xc4c, 0x0c5, 0x00a, 0xee7, 0xd85, 0x640, 0xf77, 0x333, 0x777, 0xaf6, 0x08f, 0xbbb, 0x000, 0x111, 0x222, 0x333, 0x444,
            0x555, 0x666, 0x777, 0x888, 0x999, 0xaaa, 0xbbb, 0xccc, 0xddd, 0xeee, 0xfff, 0x211, 0x433, 0x644, 0x866, 0xa88, 0xc99, 0xfbb, 0x211, 0x422, 0x633,
            0x844, 0xa55, 0xc66, 0xf77, 0x200, 0x411, 0x611, 0x822, 0xa22, 0xc33, 0xf33, 0x200, 0x400, 0x600, 0x800, 0xa00, 0xc00, 0xf00, 0x221, 0x443, 0x664,
            0x886, 0xaa8, 0xcc9, 0xfeb, 0x211, 0x432, 0x653, 0x874, 0xa95, 0xcb6, 0xfd7, 0x210, 0x431, 0x651, 0x862, 0xa82, 0xca3, 0xfc3, 0x210, 0x430, 0x640,
            0x860, 0xa80, 0xc90, 0xfb0, 0x121, 0x343, 0x564, 0x786, 0x9a8, 0xbc9, 0xdfb, 0x121, 0x342, 0x463, 0x684, 0x8a5, 0x9c6, 0xbf7, 0x120, 0x241, 0x461,
            0x582, 0x6a2, 0x8c3, 0x9f3, 0x120, 0x240, 0x360, 0x480, 0x5a0, 0x6c0, 0x7f0, 0x121, 0x343, 0x465, 0x686, 0x8a8, 0x9ca, 0xbfc, 0x121, 0x242, 0x364,
            0x485, 0x5a6, 0x6c8, 0x7f9, 0x020, 0x141, 0x162, 0x283, 0x2a4, 0x3c5, 0x3f6, 0x020, 0x041, 0x061, 0x082, 0x0a2, 0x0c3, 0x0f3, 0x122, 0x344, 0x466,
            0x688, 0x8aa, 0x9cc, 0xbff, 0x122, 0x244, 0x366, 0x488, 0x5aa, 0x6cc, 0x7ff, 0x022, 0x144, 0x166, 0x288, 0x2aa, 0x3cc, 0x3ff, 0x022, 0x044, 0x066,
            0x088, 0x0aa, 0x0cc, 0x0ff, 0x112, 0x334, 0x456, 0x668, 0x88a, 0x9ac, 0xbcf, 0x112, 0x224, 0x346, 0x458, 0x56a, 0x68c, 0x79f, 0x002, 0x114, 0x126,
            0x238, 0x24a, 0x35c, 0x36f, 0x002, 0x014, 0x016, 0x028, 0x02a, 0x03c, 0x03f, 0x112, 0x334, 0x546, 0x768, 0x98a, 0xb9c, 0xdbf, 0x112, 0x324, 0x436,
            0x648, 0x85a, 0x96c, 0xb7f, 0x102, 0x214, 0x416, 0x528, 0x62a, 0x83c, 0x93f, 0x102, 0x204, 0x306, 0x408, 0x50a, 0x60c, 0x70f, 0x212, 0x434, 0x646,
            0x868, 0xa8a, 0xc9c, 0xfbe, 0x211, 0x423, 0x635, 0x847, 0xa59, 0xc6b, 0xf7d, 0x201, 0x413, 0x615, 0x826, 0xa28, 0xc3a, 0xf3c, 0x201, 0x403, 0x604,
            0x806, 0xa08, 0xc09, 0xf0b
    };

    const int Vera::increments[32] = {
            0, 0,
            1, -1,
            2, -2,
            4, -4,
            8, -8,
            16, -16,
            32, -32,
            64, -64,
            128, -128,
            256, -256,
            512, -512,
            40, -40,
            80, -80,
            160, -160,
            320, -320,
            640, -640,
    };

    void Vera::reset() {
// init I/O registers
        std::memset(io_addr, 0, sizeof(io_addr));
        std::memset(io_inc, 0, sizeof(io_inc));
        io_addrsel = 0;
        io_dcsel = 0;
        io_rddata[0] = 0;
        io_rddata[1] = 0;

        ien = 0;
        isr = 0;
        irq_line = 0;

        // init Layer registers
        std::memset(reg_layer, 0, sizeof(reg_layer));

        // init composer registers
        std::memset(reg_composer, 0, sizeof(reg_composer));
        reg_composer[1] = 128; // hscale = 1.0
        reg_composer[2] = 128; // vscale = 1.0
        reg_composer[5] = 640 >> 2;
        reg_composer[7] = 480 >> 1;

        // Initialize FX registers
        fx_addr1_mode = 0;
        fx_x_pixel_position = 0x8000;
        fx_y_pixel_position = 0x8000;
        fx_x_pixel_increment = 0;
        fx_y_pixel_increment = 0;

        fx_cache_write = false;
        fx_cache_fill = false;
        fx_4bit_mode = false;
        fx_16bit_hop = false;
        fx_subtract = false;
        fx_cache_byte_cycling = false;
        fx_trans_writes = false;
        fx_multiplier = false;

        fx_mult_accumulator = 0;

        fx_2bit_poly = false;
        fx_2bit_poking = false;

        fx_cache_nibble_index = 0;
        fx_cache_byte_index = 0;
        fx_cache_increment_mode = 0;

        fx_cache[0] = 0;
        fx_cache[1] = 0;
        fx_cache[2] = 0;
        fx_cache[3] = 0;

        fx_16bit_hop_align = 0;

        fx_nibble_bit[0] = false;
        fx_nibble_bit[1] = false;
        fx_nibble_incr[0] = false;
        fx_nibble_incr[1] = false;

        fx_poly_fill_length = 0;
        fx_affine_tile_base = 0;
        fx_affine_map_base = 0;
        fx_affine_map_size = 2;
        fx_affine_clip = false;

        // init sprite data
        memset(sprite_data, 0, sizeof(sprite_data));

        // copy palette
        memcpy(palette, default_palette, sizeof(palette));
        for (int i = 0; i < 256; i++) {
            palette[i * 2 + 0] = default_palette[i] & 0xff;
            palette[i * 2 + 1] = default_palette[i] >> 8;
        }

        refresh_palette();

        // fill video RAM with random data
        for (int i = 0; i < 128 * 1024; i++) {
            video_ram[i] = std::rand();
        }

        sprite_line_collisions = 0;

        vga_scan_pos_x = 0;
        vga_scan_pos_y = 0;
        ntsc_half_cnt = 0;
        ntsc_scan_pos_y = 0;

        //psg_reset();
        //pcm_reset();
    }

    bool Vera::step(float mhz, float steps, bool midline) {
        WORD y = 0;
        bool ntsc_mode = reg_composer[0] & 2;
        bool new_frame = false;

        vga_scan_pos_x += PIXEL_FREQ * steps / mhz;
        if (vga_scan_pos_x > VGA_SCAN_WIDTH) {
            vga_scan_pos_x -= VGA_SCAN_WIDTH;
            if (!ntsc_mode) {
                render_line(vga_scan_pos_y - VGA_Y_OFFSET, VGA_SCAN_WIDTH);
            }
            vga_scan_pos_y++;
            if (vga_scan_pos_y == SCAN_HEIGHT) {
                vga_scan_pos_y = 0;
                if (!ntsc_mode) {
                    new_frame = true;
                    frame_count++;
                }
            }
            if (!ntsc_mode) {
                update_isr_and_coll(vga_scan_pos_y - VGA_Y_OFFSET, irq_line);
            }
        } else if (midline) {
            if (!ntsc_mode) {
                render_line(vga_scan_pos_y - VGA_Y_OFFSET, vga_scan_pos_x);
            }
        }
        ntsc_half_cnt += PIXEL_FREQ * steps / mhz;
        if (ntsc_half_cnt > NTSC_HALF_SCAN_WIDTH) {
            ntsc_half_cnt -= NTSC_HALF_SCAN_WIDTH;
            if (ntsc_mode) {
                if (ntsc_scan_pos_y < SCAN_HEIGHT) {
                    y = ntsc_scan_pos_y - NTSC_Y_OFFSET_LOW;
                    if ((y & 1) == 0) {
                        render_line(y, NTSC_HALF_SCAN_WIDTH);
                    }
                } else {
                    y = ntsc_scan_pos_y - NTSC_Y_OFFSET_HIGH;
                    if ((y & 1) == 0) {
                        render_line(y | 1, NTSC_HALF_SCAN_WIDTH);
                    }
                }
            }
            ntsc_scan_pos_y++;
            if (ntsc_scan_pos_y == SCAN_HEIGHT) {
                reg_composer[0] |= 0x80;
                if (ntsc_mode) {
                    new_frame = true;
                    frame_count++;
                }
            }
            if (ntsc_scan_pos_y == SCAN_HEIGHT * 2) {
                reg_composer[0] &= ~0x80;
                ntsc_scan_pos_y = 0;
                if (ntsc_mode) {
                    new_frame = true;
                    frame_count++;
                }
            }
            if (ntsc_mode) {
                // this is correct enough for even screen heights
                if (ntsc_scan_pos_y < SCAN_HEIGHT) {
                    update_isr_and_coll(ntsc_scan_pos_y - NTSC_Y_OFFSET_LOW, irq_line & ~1);
                } else {
                    update_isr_and_coll(ntsc_scan_pos_y - NTSC_Y_OFFSET_HIGH, irq_line & ~1);
                }
            }
        } else if (midline) {
            if (ntsc_mode) {
                if (ntsc_scan_pos_y < SCAN_HEIGHT) {
                    y = ntsc_scan_pos_y - NTSC_Y_OFFSET_LOW;
                    if ((y & 1) == 0) {
                        render_line(y, ntsc_half_cnt);
                    }
                } else {
                    y = ntsc_scan_pos_y - NTSC_Y_OFFSET_HIGH;
                    if ((y & 1) == 0) {
                        render_line(y | 1, ntsc_half_cnt);
                    }
                }
            }
        }

        return new_frame;
    }

    bool Vera::isIrq() const {
        BYTE tmp_isr = isr;//| (pcm_is_fifo_almost_empty() ? 8 : 0);
        return (tmp_isr & ien) != 0;
    }

    bool Vera::update() {
        static bool cmd_down = false;
        bool mouse_changed = false;

        // for activity LED, overlay red 8x4 square into top right of framebuffer
        // for progressive modes, draw LED only on even scanlines
        for (int y = 0; y < 4; y += 1 + !!((reg_composer[0] & 0x0b) > 0x09)) {
            for (int x = SCREEN_WIDTH - 8; x < SCREEN_WIDTH; x++) {
                BYTE b = framebuffer[(y * SCREEN_WIDTH + x) * 4 + 0];
                BYTE g = framebuffer[(y * SCREEN_WIDTH + x) * 4 + 1];
                BYTE r = framebuffer[(y * SCREEN_WIDTH + x) * 4 + 2];
                r = (DWORD) r * (255 - activity_led) / 255 + activity_led;
                g = (DWORD) g * (255 - activity_led) / 255;
                b = (DWORD) b * (255 - activity_led) / 255;
                framebuffer[(y * SCREEN_WIDTH + x) * 4 + 0] = b;
                framebuffer[(y * SCREEN_WIDTH + x) * 4 + 1] = g;
                framebuffer[(y * SCREEN_WIDTH + x) * 4 + 2] = r;
                framebuffer[(y * SCREEN_WIDTH + x) * 4 + 3] = 0x00;
            }
        }

        return true;
    }

    const BYTE* Vera::GetFrameBuffer() const {
        return framebuffer;
    }

    unsigned char Vera::read(unsigned char reg, bool debugOn) {
        bool ntsc_mode = reg_composer[0] & 2;
        WORD scanline = ntsc_mode ? ntsc_scan_pos_y % SCAN_HEIGHT : vga_scan_pos_y;
        if (scanline >= 512) scanline = 511;

        switch (reg & 0x1F) {
            case 0x00:
                return io_addr[io_addrsel] & 0xff;
            case 0x01:
                return (io_addr[io_addrsel] >> 8) & 0xff;
            case 0x02:
                return (io_addr[io_addrsel] >> 16) | (fx_nibble_bit[io_addrsel] << 1) | (fx_nibble_incr[io_addrsel] << 2) | (io_inc[io_addrsel] << 3);
            case 0x03:
            case 0x04: {
                if (debugOn) {
                    return io_rddata[reg - 3];
                }

                //bool nibble = fx_nibble_bit[reg - 3];
                DWORD address = get_and_inc_address(reg - 3, false);

                BYTE value = io_rddata[reg - 3];

                if (reg == 4 && fx_addr1_mode == 3)
                    fx_affine_prefetch();
                else
                    io_rddata[reg - 3] = video_space_read(io_addr[reg - 3]);

                if (fx_cache_fill) {
                    if (fx_4bit_mode) {
                        if (fx_cache_nibble_index) {
                            fx_cache[fx_cache_byte_index] = (fx_cache[fx_cache_byte_index] & 0xf0) | (value & 0x0f);
                            fx_cache_nibble_index = 0;
                            fx_cache_byte_index = ((fx_cache_byte_index + 1) & 0x3);
                        } else {
                            fx_cache[fx_cache_byte_index] = (fx_cache[fx_cache_byte_index] & 0x0f) | (value & 0xf0);
                            fx_cache_nibble_index = 1;
                        }
                    } else {
                        fx_cache[fx_cache_byte_index] = value;
                        if (fx_cache_increment_mode)
                            fx_cache_byte_index = (fx_cache_byte_index & 0x2) | ((fx_cache_byte_index + 1) & 0x1);
                        else
                            fx_cache_byte_index = ((fx_cache_byte_index + 1) & 0x3);
                    }
                }

                if (log_video) {
                    std::cout << std::hex << "READ  video_space[$" << address << "] = $" << value << std::endl;
                }
                return value;
            }
            case 0x05:
                return (io_dcsel << 1) | io_addrsel;
            case 0x06:
                return ((irq_line & 0x100) >> 1) | ((scanline & 0x100) >> 2) | (ien & 0xF);
            case 0x07:
                return isr;//| (pcm_is_fifo_almost_empty() ? 8 : 0);
            case 0x08:
                return scanline & 0xFF;

            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C: {
                int i = reg - 0x09 + (io_dcsel << 2);
                switch (i) {
                    case 0x00:
                    case 0x01:
                    case 0x02:
                    case 0x03:
                    case 0x04:
                    case 0x05:
                    case 0x06:
                    case 0x07:
                    case 0x08:
                        // DCSEL = [0,1] with any composer register, or [2] at $9f29
                        return reg_composer[i];
                        break;
                    case 0x16: // DCSEL=5, 0x9F2B
                        if (fx_poly_fill_length >= 768) {
                            return ((fx_2bit_poly && fx_addr1_mode == 2) ? 0x00 : 0x80);
                        }
                        if (fx_4bit_mode) {
                            if (fx_2bit_poly && fx_addr1_mode == 2) {
                                return ((fx_y_pixel_position & 0x00008000) >> 8) |
                                       ((fx_x_pixel_position >> 11) & 0x60) |
                                       ((fx_x_pixel_position >> 14) & 0x10) |
                                       ((fx_poly_fill_length & 0x0007) << 1) |
                                       ((fx_x_pixel_position & 0x00008000) >> 15);
                            } else {
                                return ((!!(fx_poly_fill_length & 0xfff8)) << 7) |
                                       ((fx_x_pixel_position >> 11) & 0x60) |
                                       ((fx_x_pixel_position >> 14) & 0x10) |
                                       ((fx_poly_fill_length & 0x0007) << 1);
                            }
                        } else {
                            return ((!!(fx_poly_fill_length & 0xfff0)) << 7) |
                                   ((fx_x_pixel_position >> 11) & 0x60) |
                                   ((fx_poly_fill_length & 0x000f) << 1);
                        }
                        break;
                    case 0x17: // DCSEL=5, 0x9F2C
                        return ((fx_poly_fill_length & 0x03f8) >> 2);
                        break;
                    case 0x18: // DCSEL=6, 0x9F29
                        fx_mult_accumulator = 0;
                        // fall out of the switch
                        break;
                    case 0x19: { // DCSEL=6, 0x9F2A
                        // <- avoids the error in some compilers about a declaration after a label
                        int32_t m_result = (int16_t) ((fx_cache[1] << 8) | fx_cache[0]) * (int16_t) ((fx_cache[3] << 8) | fx_cache[2]);
                        if (fx_subtract)
                            fx_mult_accumulator -= m_result;
                        else
                            fx_mult_accumulator += m_result;
                        // fall out of the switch
                        break;
                    }
                    default:
                        // The rest of the space is write-only,
                        // so reading the values out instead returns the version string.
                        // fall out of the switch
                        break;
                }
                return vera_version_string[i % 4];
                break;
            }
            case 0x0D:
            case 0x0E:
            case 0x0F:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
                return reg_layer[0][reg - 0x0D];

            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1A:
                return reg_layer[1][reg - 0x14];

                //case 0x1B: audio_render(); return pcm_read_ctrl();
                //case 0x1C: return pcm_read_rate();
            case 0x1D:
                return 0;

            case 0x1E:
            case 0x1F: return m_spi.read(reg & 1);
            default:
                break;
        }
        return 0;
    }

    void Vera::write(unsigned char reg, unsigned char value) {
        switch (reg & 0x1F) {
            case 0x00:
                if (fx_2bit_poly && fx_4bit_mode && fx_addr1_mode == 2 && io_addrsel == 1) {
                    fx_2bit_poking = true;
                    io_addr[1] = (io_addr[1] & 0x1fffc) | (value & 0x3);
                } else {
                    io_addr[io_addrsel] = (io_addr[io_addrsel] & 0x1ff00) | value;
                    if (fx_16bit_hop && io_addrsel == 1)
                        fx_16bit_hop_align = value & 3;
                }
                io_rddata[io_addrsel] = video_space_read(io_addr[io_addrsel]);
                break;
            case 0x01:
                io_addr[io_addrsel] = (io_addr[io_addrsel] & 0x100ff) | (value << 8);
                io_rddata[io_addrsel] = video_space_read(io_addr[io_addrsel]);
                break;
            case 0x02:
                io_addr[io_addrsel] = (io_addr[io_addrsel] & 0x0ffff) | ((value & 0x1) << 16);
                fx_nibble_bit[io_addrsel] = (value >> 1) & 0x1;
                fx_nibble_incr[io_addrsel] = (value >> 2) & 0x1;
                io_inc[io_addrsel] = value >> 3;
                io_rddata[io_addrsel] = video_space_read(io_addr[io_addrsel]);
                break;
            case 0x03:
            case 0x04: {
                if (fx_2bit_poking && fx_addr1_mode) {
                    fx_2bit_poking = false;
                    BYTE mask = value >> 6;
                    switch (mask) {
                        case 0x00:
                            video_ram[io_addr[1] & 0x1FFFF] = (fx_cache[fx_cache_byte_index] & 0xc0) | (io_rddata[1] & 0x3f);
                            break;
                        case 0x01:
                            video_ram[io_addr[1] & 0x1FFFF] = (fx_cache[fx_cache_byte_index] & 0x30) | (io_rddata[1] & 0xcf);
                            break;
                        case 0x02:
                            video_ram[io_addr[1] & 0x1FFFF] = (fx_cache[fx_cache_byte_index] & 0x0c) | (io_rddata[1] & 0xf3);
                            break;
                        case 0x03:
                            video_ram[io_addr[1] & 0x1FFFF] = (fx_cache[fx_cache_byte_index] & 0x03) | (io_rddata[1] & 0xfc);
                            break;
                    }
                    break; // break out of the enclosing switch statement early, too
                }

                bool nibble = fx_nibble_bit[reg - 3];
                DWORD address = get_and_inc_address(reg - 3, true);
                if (log_video) {
                    std::cout << std::hex << "WRITE video_space[$" << address << "] = $" << value << std::endl;
                }

                if (fx_cache_write) {
                    address &= 0x1fffc;
                    if (fx_cache_byte_cycling) {
                        fx_vram_cache_write(address + 0, fx_cache[fx_cache_byte_index], value & 0x03);
                        fx_vram_cache_write(address + 1, fx_cache[fx_cache_byte_index], (value >> 2) & 0x03);
                        fx_vram_cache_write(address + 2, fx_cache[fx_cache_byte_index], (value >> 4) & 0x03);
                        fx_vram_cache_write(address + 3, fx_cache[fx_cache_byte_index], value >> 6);
                    } else {
                        if (fx_multiplier) {
                            int32_t m_result = (int16_t) ((fx_cache[1] << 8) | fx_cache[0]) * (int16_t) ((fx_cache[3] << 8) | fx_cache[2]);
                            if (fx_subtract)
                                m_result = fx_mult_accumulator - m_result;
                            else
                                m_result = fx_mult_accumulator + m_result;
                            fx_vram_cache_write(address + 0, (m_result) & 0xff, value & 0x03);
                            fx_vram_cache_write(address + 1, (m_result >> 8) & 0xff, (value >> 2) & 0x03);
                            fx_vram_cache_write(address + 2, (m_result >> 16) & 0xff, (value >> 4) & 0x03);
                            fx_vram_cache_write(address + 3, (m_result >> 24) & 0xff, value >> 6);
                        } else {
                            fx_vram_cache_write(address + 0, fx_cache[0], value & 0x03);
                            fx_vram_cache_write(address + 1, fx_cache[1], (value >> 2) & 0x03);
                            fx_vram_cache_write(address + 2, fx_cache[2], (value >> 4) & 0x03);
                            fx_vram_cache_write(address + 3, fx_cache[3], value >> 6);
                        }
                    }
                } else {
                    if (fx_cache_byte_cycling) {
                        if (fx_4bit_mode) {
                            fx_vram_cache_write(address, fx_cache[fx_cache_byte_index], nibble + 1);
                        } else {
                            fx_vram_cache_write(address, fx_cache[fx_cache_byte_index], 0);
                        }
                    } else {
                        fx_video_space_write(address, nibble, value); // Normal write
                    }
                }

                io_rddata[reg - 3] = video_space_read(io_addr[reg - 3]);
                break;
            }
            case 0x05:
                if (value & 0x80) {
                    reset();
                }
                io_dcsel = (value >> 1) & 0x3f;
                io_addrsel = value & 1;
                break;
            case 0x06:
                irq_line = (irq_line & 0xFF) | ((value >> 7) << 8);
                ien = value & 0xF;
                break;
            case 0x07:
                isr &= value ^ 0xff;
                break;
            case 0x08:
                irq_line = (irq_line & 0x100) | value;
                break;

            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C: {
                step(MHZ, 0, true); // potential midline raster effect
                int i = reg - 0x09 + (io_dcsel << 2);
                if (i == 0) {
                    // if progressive mode field goes from 0 to 1
                    // or if mode goes from vga to something else with
                    // progressive mode on, clear the framebuffer
                    if (((reg_composer[0] & 0x8) == 0 && (value & 0x8)) ||
                        ((reg_composer[0] & 0x3) == 1 && (value & 0x3) > 1 && (value & 0x8))) {
                        std::memset(framebuffer, 0x00, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
                    }

                    // interlace field bit is read-only
                    reg_composer[0] = (reg_composer[0] & ~0x7f) | (value & 0x7f);
                    video_palette.dirty = true;
                } else {
                    reg_composer[i] = value;
                }

                switch (i) {
                    case 0x08: // DCSEL=2, $9F29
                        fx_addr1_mode = value & 0x03;
                        fx_4bit_mode = (value & 0x04) >> 2;
                        fx_16bit_hop = (value & 0x08) >> 3;
                        fx_cache_byte_cycling = (value & 0x10) >> 4;
                        fx_cache_fill = (value & 0x20) >> 5;
                        fx_cache_write = (value & 0x40) >> 6;
                        fx_trans_writes = (value & 0x80) >> 7;
                        break;
                    case 0x09: // DCSEL=2, $9F2A
                        fx_affine_tile_base = (value & 0xfc) << 9;
                        fx_affine_clip = (value & 0x02) >> 1;
                        fx_2bit_poly = (value & 0x01);
                        break;
                    case 0x0a: // DCSEL=2, $9F2B
                        fx_affine_map_base = (value & 0xfc) << 9;
                        fx_affine_map_size = 2 << ((value & 0x03) << 1);
                        break;
                    case 0x0b: // DCSEL=2, $9F2C
                        fx_cache_increment_mode = value & 0x01;
                        fx_cache_nibble_index = (value & 0x02) >> 1;
                        fx_cache_byte_index = (value & 0x0c) >> 2;
                        fx_multiplier = (value & 0x10) >> 4;
                        fx_subtract = (value & 0x20) >> 5;
                        if (value & 0x40) { // accumulate
                            int32_t m_result = (int16_t) ((fx_cache[1] << 8) | fx_cache[0]) * (int16_t) ((fx_cache[3] << 8) | fx_cache[2]);
                            if (fx_subtract)
                                fx_mult_accumulator -= m_result;
                            else
                                fx_mult_accumulator += m_result;
                        }
                        if (value & 0x80) { // reset accumulator
                            fx_mult_accumulator = 0;
                        }
                        break;
                    case 0x0c: // DCSEL=3, $9F29
                        fx_x_pixel_increment = ((((reg_composer[0x0d] & 0x7f) << 15) + (reg_composer[0x0c] << 7)) // base value
                                                | ((reg_composer[0x0d] & 0x40) ? 0xffc00000 : 0)) // sign extend if negative
                                << 5 * (!!(reg_composer[0x0d] & 0x80)); // multiply by 32 if flag set
                        break;
                    case 0x0d: // DCSEL=3, $9F2A
                        fx_x_pixel_increment = ((((reg_composer[0x0d] & 0x7f) << 15) + (reg_composer[0x0c] << 7)) // base value
                                                | ((reg_composer[0x0d] & 0x40) ? 0xffc00000 : 0)) // sign extend if negative
                                << 5 * (!!(reg_composer[0x0d] & 0x80)); // multiply by 32 if flag set
                        // Reset subpixel to 0.5
                        fx_x_pixel_position = (fx_x_pixel_position & 0x07ff0000) | 0x00008000;
                        break;
                    case 0x0e: // DCSEL=3, $9F2B
                        fx_y_pixel_increment = ((((reg_composer[0x0f] & 0x7f) << 15) + (reg_composer[0x0e] << 7)) // base value
                                                | ((reg_composer[0x0f] & 0x40) ? 0xffc00000 : 0)) // sign extend if negative
                                << 5 * (!!(reg_composer[0x0f] & 0x80)); // multiply by 32 if flag set
                        break;
                    case 0x0f: // DCSEL=3, $9F2C
                        fx_y_pixel_increment = ((((reg_composer[0x0f] & 0x7f) << 15) + (reg_composer[0x0e] << 7)) // base value
                                                | ((reg_composer[0x0f] & 0x40) ? 0xffc00000 : 0)) // sign extend if negative
                                << 5 * (!!(reg_composer[0x0f] & 0x80)); // multiply by 32 if flag set
                        // Reset subpixel to 0.5
                        fx_y_pixel_position = (fx_y_pixel_position & 0x07ff0000) | 0x00008000;
                        break;
                    case 0x10: // DCSEL=4, $9F29
                        fx_x_pixel_position = (fx_x_pixel_position & 0x0700ff80) | (value << 16);
                        fx_affine_prefetch();
                        break;
                    case 0x11: // DCSEL=4, $9F2A
                        fx_x_pixel_position = (fx_x_pixel_position & 0x00ffff00) | ((value & 0x7) << 24) | (value & 0x80);
                        fx_affine_prefetch();
                        break;
                    case 0x12: // DCSEL=4, $9F2B
                        fx_y_pixel_position = (fx_y_pixel_position & 0x0700ff80) | (value << 16);
                        fx_affine_prefetch();
                        break;
                    case 0x13: // DCSEL=4, $9F2C
                        fx_y_pixel_position = (fx_y_pixel_position & 0x00ffff00) | ((value & 0x7) << 24) | (value & 0x80);
                        fx_affine_prefetch();
                        break;
                    case 0x14: // DCSEL=5, $9F29
                        fx_x_pixel_position = (fx_x_pixel_position & 0x07ff0080) | (value << 8);
                        break;
                    case 0x15: // DCSEL=5, $9F2A
                        fx_y_pixel_position = (fx_y_pixel_position & 0x07ff0080) | (value << 8);
                        break;
                    case 0x18: // DCSEL=6, $9F29
                        fx_cache[0] = value;
                        break;
                    case 0x19: // DCSEL=6, $9F2A
                        fx_cache[1] = value;
                        break;
                    case 0x1a: // DCSEL=6, $9F2B
                        fx_cache[2] = value;
                        break;
                    case 0x1b: // DCSEL=6, $9F2C
                        fx_cache[3] = value;
                        break;
                }
                break;
            }

            case 0x0D:
            case 0x0E:
            case 0x0F:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
                step(MHZ, 0, true); // potential midline raster effect
                reg_layer[0][reg - 0x0D] = value;
                refresh_layer_properties(0);
                break;

            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1A:
                step(MHZ, 0, true); // potential midline raster effect
                reg_layer[1][reg - 0x14] = value;
                refresh_layer_properties(1);
                break;

                //case 0x1B: audio_render(); pcm_write_ctrl(value); break;
                //case 0x1C: audio_render(); pcm_write_rate(value); break;
                //case 0x1D: audio_render(); pcm_write_fifo(value); break;

            case 0x1E:
            case 0x1F:
                m_spi.write(reg & 1, value);
                break;
        }
    }
}
