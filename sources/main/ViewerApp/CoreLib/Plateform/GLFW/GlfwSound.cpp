//
// Created by pierr on 21/10/2023.
//


#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "GlfwSound.h"
#include "Commons/Log.h"
#include "Commons/AstraException.h"

namespace Astra::UI::Core::Glfw {
    GlfwSound::GlfwSound(int frequency) {
        LOG_DEBUG("[GlfwSound] Init {}", frequency);

        m_audioConfig = ma_device_config_init(ma_device_type_playback);

        m_audioConfig.playback.format = ma_format_f32;
        m_audioConfig.playback.channels = 1;
        m_audioConfig.sampleRate = frequency;

        m_audioConfig.dataCallback = [](ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
            const auto& callback = *(std::function<void(float*, int)>*) pDevice->pUserData;
            callback((float*) pOutput, (int) frameCount);
        };

        LOG_DEBUG("[GlfwSound] Init END");
    }

    GlfwSound::~GlfwSound() {
        if (m_isInit) {
            ma_device_uninit(&m_audioDevice);
        }
    }

    void GlfwSound::SetPauseAudio(bool isPaused) {
        if (!m_isInit) {
            return;
        }

        if (isPaused) {
            ma_device_stop(&m_audioDevice);
        } else {
            ma_device_start(&m_audioDevice);
        }

        m_isSoundPaused = isPaused;
    }

    void GlfwSound::SetDataCallback(std::function<void(float*, int)>&& callback) {
        if (m_isInit) {
            return;
        }

        m_callback = std::move(callback);
        m_audioConfig.pUserData = &m_callback;

        auto res = ma_device_init(nullptr, &m_audioConfig, &m_audioDevice);
        AstraException::assertV(res == MA_SUCCESS, "[GlfwSound] Can not init audio device");

        m_isInit = true;
    }

    bool GlfwSound::SetMasterVolume(float volume) {
        if (!m_isInit || volume < 0 || volume > 1) {
            return false;
        }

        return ma_device_set_master_volume(&m_audioDevice, volume) == MA_SUCCESS;
    }
}
