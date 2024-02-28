//
// Created by pierr on 29/07/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APopup.h"

namespace Astra::UI::Core {

    class SimpleInputPopup : public APopup
    {
    private:
        int m_category = 0;
        bool m_isValid = false;
        char m_buffer[512] = {0};

    public:
        explicit SimpleInputPopup(std::string&& name) : APopup(std::move(name)) {}

        void OpenMessage(int category = 0, const std::string_view& defaultMsg = "");

        const char* getValue();
        inline int getCategory() const {return m_category;}

    private:
        void drawPopupContent() override;
    };

}
