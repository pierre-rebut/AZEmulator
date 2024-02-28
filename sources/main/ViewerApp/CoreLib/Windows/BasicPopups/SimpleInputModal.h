//
// Created by pierr on 28/07/2023.
//
#pragma once

#include <string>
#include <functional>

#include "ViewerApp/CoreLib/Resources/Texture.h"
#include "ViewerApp/Custom/ViewerResources.h"
#include "ViewerApp/CoreLib/Windows/APopup.h"

namespace Astra::UI::Core {

    class SimpleInputModal : public AModal
    {
    private:
        std::string m_msg = "undefined";
        Ref<UI::Core::Texture> m_icon;
        std::function<void(const char*)> m_callback;

        char m_buffer[1024] = {0};

    public:
        explicit SimpleInputModal(std::string&& name) : AModal(std::move(name)) {}

        void OpenInput(const std::string_view&, const std::string_view&, std::function<void(const char*)>&&, const Ref<UI::Core::Texture>& = App::ViewerResources::InfoIcon);

    private:
        void drawPopupContent() override;
    };

}
