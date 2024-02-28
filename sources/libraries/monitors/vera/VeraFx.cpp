//
// Created by pierr on 17/01/2024.
//

#include "Vera.h"

namespace Astra::CPU::Lib::Monitors {

    void Vera::fx_affine_prefetch() {
        if (fx_addr1_mode != 3) return; // only if affine mode is selected

        DWORD address;
        BYTE affine_x_tile = (fx_x_pixel_position >> 19) & 0xff;
        BYTE affine_y_tile = (fx_y_pixel_position >> 19) & 0xff;
        BYTE affine_x_sub_tile = (fx_x_pixel_position >> 16) & 0x07;
        BYTE affine_y_sub_tile = (fx_y_pixel_position >> 16) & 0x07;

        if (!fx_affine_clip) { // wrap
            affine_x_tile &= fx_affine_map_size - 1;
            affine_y_tile &= fx_affine_map_size - 1;
        }

        if (affine_x_tile >= fx_affine_map_size || affine_y_tile >= fx_affine_map_size) {
            // We clipped, return value for tile 0
            address = fx_affine_tile_base + (affine_y_sub_tile << (3 - fx_4bit_mode)) + (affine_x_sub_tile >> (BYTE) fx_4bit_mode);
            if (fx_4bit_mode) fx_nibble_bit[1] = 0;
        } else {
            // Get the address within the tile map
            address = fx_affine_map_base + (affine_y_tile * fx_affine_map_size) + affine_x_tile;
            // Now translate that to the tile base address
            BYTE affine_tile_idx = video_space_read(address);
            address = fx_affine_tile_base + (affine_tile_idx << (6 - fx_4bit_mode));
            // Now add the sub-tile address
            address += (affine_y_sub_tile << (3 - fx_4bit_mode)) + (affine_x_sub_tile >> (BYTE) fx_4bit_mode);
            if (fx_4bit_mode) fx_nibble_bit[1] = affine_x_sub_tile & 1;
        }
        io_addr[1] = address;
        io_rddata[1] = video_space_read(address);
    }

    void Vera::fx_video_space_write(DWORD address, bool nibble, BYTE value) {
        if (fx_4bit_mode) {
            if (nibble) {
                if (!fx_trans_writes || (value & 0x0f) > 0) {
                    video_ram[address & 0x1FFFF] = (video_ram[address & 0x1FFFF] & 0xf0) | (value & 0x0f);
                }
            } else {
                if (!fx_trans_writes || (value & 0xf0) > 0) {
                    video_ram[address & 0x1FFFF] = (video_ram[address & 0x1FFFF] & 0x0f) | (value & 0xf0);
                }
            }
        } else {
            if (!fx_trans_writes || value > 0) video_ram[address & 0x1FFFF] = value;
        }
        if (address >= ADDR_PSG_START && address < ADDR_PSG_END) {
            //audio_render();
            //psg_writereg(address & 0x3f, value);
        } else if (address >= ADDR_PALETTE_START && address < ADDR_PALETTE_END) {
            palette[address & 0x1ff] = value;
            video_palette.dirty = true;
        } else if (address >= ADDR_SPRDATA_START && address < ADDR_SPRDATA_END) {
            sprite_data[(address >> 3) & 0x7f][address & 0x7] = value;
            refresh_sprite_properties((address >> 3) & 0x7f);
        }
    }

    void Vera::fx_vram_cache_write(DWORD address, BYTE value, BYTE mask) {
        if (!fx_trans_writes || value > 0) {
            switch (mask) {
                case 0:
                    video_ram[address & 0x1FFFF] = value;
                    break;
                case 1:
                    video_ram[address & 0x1FFFF] = (video_ram[address & 0x1FFFF] & 0x0f) | (value & 0xf0);
                    break;
                case 2:
                    video_ram[address & 0x1FFFF] = (video_ram[address & 0x1FFFF] & 0xf0) | (value & 0x0f);
                    break;
                case 3:
                    // Do nothing
                    break;
            }
        }
    }

}
