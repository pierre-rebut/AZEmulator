//
// Created by pierr on 17/01/2024.
//
#pragma once

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define VERA_VERSION_MAJOR  0x00
#define VERA_VERSION_MINOR  0x03
#define VERA_VERSION_PATCH  0x02

#define ADDR_VRAM_START     0x00000
#define ADDR_VRAM_END       0x20000
#define ADDR_PSG_START      0x1F9C0
#define ADDR_PSG_END        0x1FA00
#define ADDR_PALETTE_START  0x1FA00
#define ADDR_PALETTE_END    0x1FC00
#define ADDR_SPRDATA_START  0x1FC00
#define ADDR_SPRDATA_END    0x20000

#define NUM_SPRITES 128
#define NUM_LAYERS 2

// both VGA and NTSC
#define SCAN_HEIGHT 525
#define PIXEL_FREQ 25.0

// VGA
#define VGA_SCAN_WIDTH 800
#define VGA_Y_OFFSET 0

// NTSC: 262.5 lines per frame, lower field first
#define NTSC_HALF_SCAN_WIDTH 794
#define NTSC_X_OFFSET 270
#define NTSC_Y_OFFSET_LOW 42
#define NTSC_Y_OFFSET_HIGH 568
#define TITLE_SAFE_X 0.067
#define TITLE_SAFE_Y 0.05

#define MAX(a, b) ((a) > (b) ? a : b)