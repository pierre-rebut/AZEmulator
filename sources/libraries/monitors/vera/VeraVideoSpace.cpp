//
// Created by pierr on 17/01/2024.
//

#include <cstring>
#include "Vera.h"

namespace Astra::CPU::Lib::Monitors {
    BYTE Vera::video_space_read(DWORD address) const {
        return video_ram[address & 0x1FFFF];
    }

    void Vera::video_space_read_range(BYTE* dest, DWORD address, DWORD size) const {
        if (address >= ADDR_VRAM_START && (address + size) <= ADDR_VRAM_END) {
            std::memcpy(dest, &video_ram[address], size);
        } else {
            for (int i = 0; i < size; ++i) {
                *dest++ = video_space_read(address + i);
            }
        }
    }

    void Vera::video_space_write(DWORD address, BYTE value) {
        video_ram[address & 0x1FFFF] = value;

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
}
