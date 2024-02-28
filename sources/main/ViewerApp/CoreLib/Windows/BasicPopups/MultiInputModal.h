//
// Created by pierr on 27/08/2023.
//

#pragma once

#include <string>
#include <vector>
#include <functional>

#include "ViewerApp/CoreLib/Windows/APopup.h"

namespace Astra::UI::Core {
    class MultiInputModal : public AModal
    {
    public:
        using OptionType = std::vector<std::pair<const std::string, std::string>>;

    private:
        std::string m_msg = "undefined";
        OptionType m_options;
        std::function<bool(const OptionType& result)> m_callback;

    public:
        explicit MultiInputModal(std::string&& name) : AModal(std::move(name)) {}

        void OpenInput(std::string&& msg, OptionType&& options, std::function<bool(const OptionType&)>&& callback);

    private:
        void drawPopupContent() override;
    };
}