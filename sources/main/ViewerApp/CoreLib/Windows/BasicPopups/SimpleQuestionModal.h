//
// Created by pierr on 10/08/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APopup.h"

#include <functional>

namespace Astra::UI::Core {

    class SimpleQuestionModal : public AModal
    {
    private:
        std::string m_msg = "undefined";
        std::function<void(bool)> m_callback;

    public:
        explicit SimpleQuestionModal(std::string&& name) : AModal(std::move(name)) {}

        void OpenQuestion(std::string&& question, std::function<void(bool)>&& callback);

    private:
        void drawPopupContent() override;
    };

}
