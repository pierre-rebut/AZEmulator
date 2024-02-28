//
// Created by pierr on 17/01/2024.
//
#pragma once

#include <cstdint>

#include "Spi.h"
#include "VideoStruct.h"
#include "VeraDefines.h"

namespace Astra::CPU::Lib::Monitors {

    class Vera
    {
    private:
        static const BYTE vera_version_string[];
        static const WORD default_palette[];
        static const int increments[32];

        Spi& m_spi;

        int MHZ = 8;
        BYTE activity_led = 0;

        BYTE video_ram[0x20000];
        BYTE palette[256 * 2];
        BYTE sprite_data[128][8];

// I/O registers
        DWORD io_addr[2];
        BYTE io_rddata[2];
        BYTE io_inc[2];
        BYTE io_addrsel;
        BYTE io_dcsel;

        BYTE ien;
        BYTE isr;

        WORD irq_line;

        BYTE reg_layer[2][7];

#define COMPOSER_SLOTS 4*64
        BYTE reg_composer[COMPOSER_SLOTS];
        BYTE prev_reg_composer[2][COMPOSER_SLOTS];

        BYTE layer_line[2][SCREEN_WIDTH];
        BYTE sprite_line_col[SCREEN_WIDTH];
        BYTE sprite_line_z[SCREEN_WIDTH];
        BYTE sprite_line_mask[SCREEN_WIDTH];
        BYTE sprite_line_collisions;
        bool layer_line_enable[2];
        bool old_layer_line_enable[2];
        bool old_sprite_line_enable;
        bool sprite_line_enable;

////////////////////////////////////////////////////////////
// FX registers
////////////////////////////////////////////////////////////
        BYTE fx_addr1_mode;

// These are all 16.16 fixed point in the emulator
// even though the VERA uses smaller bit widths
// for the whole and fractional parts.
//
// Sign extension is done manually when assigning negative numbers
//
// Native VERA bit widths are shown below.
        DWORD fx_x_pixel_increment;  // 11.9 fixed point (6.9 without 32x multiplier, 11.4 with 32x multiplier on)
        DWORD fx_y_pixel_increment;  // 11.9 fixed point (6.9 without 32x multiplier, 11.4 with 32x multiplier on)
        DWORD fx_x_pixel_position;   // 11.9 fixed point
        DWORD fx_y_pixel_position;   // 11.9 fixed point

        WORD fx_poly_fill_length;      // 10 bits

        DWORD fx_affine_tile_base;
        DWORD fx_affine_map_base;

        BYTE fx_affine_map_size;

        bool log_video = false;

        bool fx_4bit_mode;
        bool fx_16bit_hop;
        bool fx_cache_byte_cycling;
        bool fx_cache_fill;
        bool fx_cache_write;
        bool fx_trans_writes;

        bool fx_2bit_poly;
        bool fx_2bit_poking;

        bool fx_cache_increment_mode;
        bool fx_cache_nibble_index;
        BYTE fx_cache_byte_index;
        bool fx_multiplier;
        bool fx_subtract;

        bool fx_affine_clip;

        BYTE fx_16bit_hop_align;

        bool fx_nibble_bit[2];
        bool fx_nibble_incr[2];

        BYTE fx_cache[4];

        int32_t fx_mult_accumulator;

        float vga_scan_pos_x;
        WORD vga_scan_pos_y;
        float ntsc_half_cnt;
        WORD ntsc_scan_pos_y;
        int frame_count = 0;

        BYTE framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT * 4];

        video_layer_properties layer_properties[NUM_LAYERS];
        video_layer_properties prev_layer_properties[2][NUM_LAYERS];
        video_sprite_properties sprite_properties[128];
        video_palette video_palette;

    public:
        explicit Vera(Spi& spi) : m_spi(spi) {}

        void reset();
        bool step(float mhz, float steps, bool midline);
        bool isIrq() const;
        bool update();
        const BYTE* GetFrameBuffer() const;
        BYTE read(BYTE reg, bool debugOn);
        void write(BYTE reg, BYTE value);

    private:
        void refresh_palette();

        void refresh_layer_properties(const BYTE layer);
        void refresh_sprite_properties(const WORD sprite);
        void render_sprite_line(const WORD y);
        void render_layer_line_text(BYTE layer, WORD y);
        void render_layer_line_tile(BYTE layer, WORD y);
        void render_layer_line_bitmap(BYTE layer, WORD y);
        void render_line(WORD y, float scan_pos_x);
        void update_isr_and_coll(WORD y, WORD compare);
        DWORD get_and_inc_address(BYTE sel, bool write);

        // FX
        void fx_affine_prefetch();
        void fx_video_space_write(DWORD address, bool nibble, BYTE value);
        void fx_vram_cache_write(DWORD address, BYTE value, BYTE mask);

        // Internal Video Address Space
        BYTE video_space_read(DWORD address) const;
        void video_space_read_range(BYTE* dest, DWORD address, DWORD size) const;
        void video_space_write(DWORD address, BYTE value);
    };

}
