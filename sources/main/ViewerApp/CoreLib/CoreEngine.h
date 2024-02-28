//
// Created by pierr on 15/03/2023.
//
#pragma once

#include "Commons/utils/Singleton.h"
#include "ViewerApp/CoreLib/Graphics/ImGuiDevice.h"
#include "ViewerApp/CoreLib/Resources/Resources.h"

#include "EngineLib/data/Base.h"
#include "ViewerApp/CoreLib/Assets/AssetManager.h"
#include "MainScreen.h"
#include "AsyncJob.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/TipsText.h"
#include "ViewerApp/CoreLib/System/I18N.h"

#include <queue>

namespace Astra::UI::Core {

    class CoreEngine : public Singleton<CoreEngine>
    {
    public:
        static constexpr const char* NAME = "CoreEngine";

    private:
        Scope<AWindow> m_window;
        Scope<ImGuiDevice> m_device;
        Scope<AssetManager> m_assetManager;
        Scope<MainScreen> m_mainScreen;
        Scope<AsyncJob> m_asyncJob;
        Scope<TipsText> m_tipsText;
        Scope<I18NImpl> m_i18n;

        uint64_t m_frameCount = 0;

    public:
        CoreEngine();
        ~CoreEngine() override;

        template<class T = MainScreen>
        void initScreen() {
            m_mainScreen = CreateScope<T>();
            m_mainScreen->init();
        }

        void Run();

        AWindow& GetWindow() const { return *m_window; }

    private:
        static void initImGui();

        static void setDarkThemeColors();
    };

}
