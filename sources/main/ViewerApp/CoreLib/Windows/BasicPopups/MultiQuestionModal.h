//
// Created by pierr on 23/08/2023.
//

#pragma once

#include <string>
#include <vector>
#include <functional>

#include "ViewerApp/CoreLib/Windows/APopup.h"

namespace Astra::UI::Core {

    class MultiQuestionModal : public AModal
    {
    private:
        std::string m_msg = "undefined";
        std::vector<std::string> m_options;
        std::function<void(int)> m_callback;

    public:
        explicit MultiQuestionModal(std::string&& name) : AModal(std::move(name)) {}

        void OpenQuestion(std::string&& question, std::vector<std::string>&& options, std::function<void(int)>&& callback);

    private:
        void drawPopupContent() override;
    };

} // Astra
