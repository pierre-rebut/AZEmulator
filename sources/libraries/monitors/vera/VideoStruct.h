//
// Created by pierr on 17/01/2024.
//
#pragma once

#include <cstdint>
#include "EngineLib/data/Types.h"

namespace Astra::CPU::Lib::Monitors {

    struct video_layer_properties
    {
        BYTE color_depth;
        DWORD map_base;
        DWORD tile_base;

        bool text_mode;
        bool text_mode_256c;
        bool tile_mode;
        bool bitmap_mode;

        WORD hscroll;
        WORD vscroll;

        BYTE mapw_log2;
        BYTE maph_log2;
        WORD tilew;
        WORD tileh;
        BYTE tilew_log2;
        BYTE tileh_log2;

        WORD mapw_max;
        WORD maph_max;
        WORD tilew_max;
        WORD tileh_max;
        WORD layerw_max;
        WORD layerh_max;

        BYTE tile_size_log2;

        int min_eff_x;
        int max_eff_x;

        BYTE bits_per_pixel;
        BYTE first_color_pos;
        BYTE color_mask;
        BYTE color_fields_max;
    };

    struct video_sprite_properties
    {
        int8_t sprite_zdepth;
        BYTE sprite_collision_mask;

        int16_t sprite_x;
        int16_t sprite_y;
        BYTE sprite_width_log2;
        BYTE sprite_height_log2;
        BYTE sprite_width;
        BYTE sprite_height;

        bool hflip;
        bool vflip;

        BYTE color_mode;
        DWORD sprite_address;

        WORD palette_offset;
    };

    struct video_palette
    {
        DWORD entries[256];
        bool dirty;
    };

}
