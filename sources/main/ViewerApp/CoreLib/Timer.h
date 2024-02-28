//
// Created by pierr on 04/03/2022.
//

#pragma once

// std
#include <chrono>

namespace Astra::UI::Core {

    class Timer
    {
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;

    public:
        Timer() {
            reset();
        }

        void reset() {
            m_start = std::chrono::high_resolution_clock::now();
        }

        float get() const {
            return std::chrono::duration<float, std::chrono::seconds::period>(
                    std::chrono::high_resolution_clock::now() - m_start
            ).count();
        }

    };
}
