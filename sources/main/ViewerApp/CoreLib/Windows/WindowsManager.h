//
// Created by pierr on 26/06/2023.
//
#pragma once

#include "Commons/utils/Singleton.h"
#include "Commons/utils/ObjectStack.h"

#include "APanel.h"
#include "APopup.h"

namespace Astra::UI::Core {

    class WindowsManager : public Singleton<WindowsManager>
    {
    public:
        static constexpr const char* NAME = "WindowsManager";

    private:
        ObjectStack<APanel> m_panelsStack;
        ObjectStack<APopup> m_popupsStack;

    public:
        void OnUpdate(const UI::Core::FrameInfo& pFrameInfo) const;

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) const;

        void OnEvent(UI::Core::AEvent& pEvent) const;

        ObjectStack<APanel>& getPanels() { return m_panelsStack; }

        ObjectStack<APopup>& getPopups() { return m_popupsStack; }
    };

}
