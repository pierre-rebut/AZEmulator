//
// Created by pierr on 28/07/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APopup.h"
#include "ViewerApp/CoreLib/Resources/Texture.h"
#include "ViewerApp/Custom/ViewerResources.h"

#include <vector>

namespace Astra::UI::Core {

    class TextModal : public AModal
    {
    private:
        std::vector<std::string> m_msgs;
        Ref<UI::Core::Texture> m_icon;

    public:
        explicit TextModal(std::string&& name) : AModal(std::move(name)) {  }

        void OpenMessage(std::vector<std::string>&& msg, const Ref<UI::Core::Texture>& icon = App::ViewerResources::InfoIcon);

    private:
        void drawPopupContent() override;
    };

}
