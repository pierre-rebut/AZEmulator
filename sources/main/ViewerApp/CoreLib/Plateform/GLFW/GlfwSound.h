//
// Created by pierr on 21/10/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Resources/Sound.h"
#include "miniaudio.h"

namespace Astra::UI::Core::Glfw {

    class GlfwSound : public Sound
    {
    private:
        bool m_isInit = false;
        ma_device_config m_audioConfig;
        ma_device m_audioDevice;

    public:
        explicit GlfwSound(int frequency);
        ~GlfwSound() override;

        void SetPauseAudio(bool isPaused) override;
        void SetDataCallback(std::function<void(float*, int)>&& callback) override;
        bool SetMasterVolume(float volume) override;
    };

}
