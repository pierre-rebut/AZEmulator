//
// Created by pierr on 26/06/2023.
//
#pragma once

#include <string>

#include "Commons/AObject.h"
#include "ViewerApp/CoreLib/Events/AEvent.h"

namespace Astra::UI::Core {

    class APopup : public AObject
    {
    public:
        const std::string Name;

    protected:
        bool m_isOpen = false;

    public:
        explicit APopup(std::string pName);
        void Open();
        virtual void OnImGuiRender();
        virtual void Reset() { /* do nothing */ }

        virtual void OnEvent(const AEvent& pEvent) { /* do nothing */ }

        std::string toString() const override;

    protected:
        virtual void drawPopupContent() = 0;
    };

    class AModal : public APopup
    {
    public:
        using APopup::APopup;

        void OnImGuiRender() override;
    };

}
