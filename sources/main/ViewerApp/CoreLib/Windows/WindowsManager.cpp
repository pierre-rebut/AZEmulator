//
// Created by pierr on 26/06/2023.
//
#include "WindowsManager.h"

#include "Commons/Profiling.h"

namespace Astra::UI::Core {

    void WindowsManager::OnUpdate(const UI::Core::FrameInfo& pFrameInfo) const {
        ENGINE_PROFILE_FUNCTION();

        for (const auto& [_, panel]: m_panelsStack) {
            panel->OnUpdate(pFrameInfo);
        }
    }

    void WindowsManager::OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) const {
        ENGINE_PROFILE_FUNCTION();

        for (const auto& [_, panel]: m_panelsStack) {
            panel->OnImGuiRender(pFrameInfo);
        }
    }

    void WindowsManager::OnEvent(UI::Core::AEvent& pEvent) const {
        ENGINE_PROFILE_FUNCTION();

        for (const auto& [_, panel]: m_panelsStack) {
            if (pEvent.Handled) {
                return;
            }

            panel->OnEvent(pEvent);
        }

        for (const auto& [_, popup]: m_popupsStack) {
            if (pEvent.Handled) {
                return;
            }

            popup->OnEvent(pEvent);
        }
    }
}
