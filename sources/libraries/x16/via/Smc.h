//
// Created by pierr on 13/01/2024.
//
#pragma once

#include "EngineLib/data/Types.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"
#include "EngineLib/data/KeyCodes.h"

#include <array>

namespace Astra::CPU::Lib::X16 {

    static constexpr const int BUFFER_SIZE = 16;

    class Smc
    {
    public:
        static constexpr const int EXTENDED_FLAG = 0x100;

    private:
        const Ref<IDevice>& m_keyboard;
        const Ref<IDevice>& m_mouse;

        bool& sendNmi;

        BYTE activity_led = 0;
        BYTE mse_count = 0;
        bool smc_requested_reset = false;
        BYTE mouse_device_id = 0;

        class RingBuffer {
        private:
            BYTE head = 0;
            BYTE tail = 0;
            std::array<BYTE, BUFFER_SIZE> buffer{};

        public:
            void add(BYTE value);
            BYTE next();
            void reset();
            int count() const;
        };

        RingBuffer keyboard;
        RingBuffer mouse;

    public:
        Smc(bool& a, const Ref<IDevice>& kbr, const Ref<IDevice>& mse) : sendNmi(a), m_keyboard(kbr), m_mouse(mse) {}

        void step();

        void reset();
        BYTE read(BYTE a);
        void write(BYTE a, BYTE v);

    private:
        static int keyCodeToX16KeyCode(KeyCode code);
        void mouseSend(int x, int y, BYTE btn, char wheel);
    };

}
