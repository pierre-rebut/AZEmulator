//
// Created by pierr on 20/10/2023.
//
#pragma once

#include <functional>
#include <cstdint>

#include "EngineLib/data/Base.h"

namespace Astra::UI::Core {

    class Sound
    {
    protected:
        bool m_isSoundPaused = true;
        std::function<void(float*, int)> m_callback;

    public:
        virtual ~Sound() = default;

        virtual void SetPauseAudio(bool isPaused) = 0;
        virtual void SetDataCallback(std::function<void(float*, int)>&& callback) = 0;
        virtual bool SetMasterVolume(float volume) = 0;

        bool IsSoundPaused() const {return m_isSoundPaused;}

        static Scope<Sound> CreateSound(int frequency);
    };

}
