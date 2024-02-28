//
// Created by pierr on 27/08/2023.
//

#pragma once

#include <list>

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "ViewerApp/Custom/Panels/Views/Nodes/NodeItem.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "CpuEngine/engine/Device.h"

namespace Astra::UI::App {

    class NodesPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = ICON_FA_SITEMAP " Nodes";

    private:
        std::list<Ref<NodeItem>> m_nodeGraph;
        bool m_isAppearing = true;

    public:
        NodesPanel() : UI::Core::APanel(NAME, UI::Core::PanelType::VIEW) {}

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) override;
        void OnUpdate(const UI::Core::FrameInfo& pFrameInfo) override;

    private:
        void drawPanelContent() override;
        static int getIndexByUUID(UUID uuid);
        static const char* getIconByDeviceType(CPU::Core::DeviceType type) ;
    };

} // Astra
