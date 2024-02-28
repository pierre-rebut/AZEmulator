//
// Created by pierr on 15/10/2023.
//
#pragma once

#include <cstdint>

namespace Astra::CPU::Lib {

    constexpr uint8_t  nDefaultAlpha = 0xFF;
    constexpr uint32_t nDefaultPixel = (nDefaultAlpha << 24);

    struct Pixel
    {
        union
        {
            uint32_t n = nDefaultPixel;
            struct { uint8_t r; uint8_t g; uint8_t b; uint8_t a; };
        };

        enum Mode { NORMAL, MASK, ALPHA, CUSTOM };

        Pixel();
        Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = nDefaultAlpha);
        Pixel(uint32_t p);
        Pixel& operator = (const Pixel& v) = default;
        bool   operator ==(const Pixel& p) const;
        bool   operator !=(const Pixel& p) const;
        Pixel  operator * (const float i) const;
        Pixel  operator / (const float i) const;
        Pixel& operator *=(const float i);
        Pixel& operator /=(const float i);
        Pixel  operator + (const Pixel& p) const;
        Pixel  operator - (const Pixel& p) const;
        Pixel& operator +=(const Pixel& p);
        Pixel& operator -=(const Pixel& p);
        Pixel  operator * (const Pixel& p) const;
        Pixel& operator *=(const Pixel& p);
        Pixel  inv() const;
    };

}
