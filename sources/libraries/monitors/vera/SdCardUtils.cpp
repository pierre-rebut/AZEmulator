//
// Created by pierr on 19/01/2024.
//

#include "SdCard.h"

namespace Astra::CPU::Lib::Monitors {

    void SdCard::set_response_csd() {
        size_t c_size = (sdCardFile.size() >> 19) - 1;
        rr[12] |= (c_size >> 16) & 0x3f;
        rr[13] = (c_size >> 8) & 0xff;
        rr[14] = c_size & 0xff;

        response = rr.data();
        response_length = 21;
    }

    void SdCard::set_response_r1() {
        static BYTE r1;
        r1 = is_idle ? 1 : 0;
        response = &r1;
        response_length = 1;
    }

    void SdCard::set_response_r2() {
        if (is_initialized) {
            static const BYTE r2[] = {0x00, 0x00};
            response = r2;
            response_length = sizeof(r2);
        } else {
            static const BYTE r2[] = {0x1F, 0xFF};
            response = r2;
            response_length = sizeof(r2);
        }
    }

    void SdCard::set_response_r3() {
        static const BYTE r3[] = {0xC0, 0xFF, 0x80, 0x00};
        response = r3;
        response_length = sizeof(r3);
    }

    void SdCard::set_response_r7() {
        static const BYTE r7[] = {1, 0x00, 0x00, 0x01, 0xAA};
        response = r7;
        response_length = sizeof(r7);
    }
}
