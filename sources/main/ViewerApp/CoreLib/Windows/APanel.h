//
// Created by pierr on 05/03/2022.
//

#pragma once

#include "Commons/AObject.h"
#include "ViewerApp/CoreLib/FrameInfo.h"
#include "ViewerApp/CoreLib/Events/AEvent.h"

#include <string>

namespace Astra::UI::Core {

    enum class PanelType
    {
        NONE,
        VIEW,
        TOOLS
    };

    class APanel : public AObject
    {
    protected:
        bool m_isOpen = true;

    private:
        virtual void drawPanelContent() = 0;

    public:
        const std::string Name;
        const PanelType Type;

        APanel(std::string pName, PanelType pType);

        virtual void OnImGuiRender(const FrameInfo& pFrameInfo);

        virtual void OnUpdate(const FrameInfo& pFrameInfo) { /* do nothing */ }

        virtual void OnEvent(AEvent& pEvent) { /* do nothing */ }

        bool IsOpen() const { return m_isOpen; }

        void Open();

        std::string toString() const override;
    };
}
