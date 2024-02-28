//
// Created by pierr on 11/03/2023.
//
#pragma once

#include <string>
#include <sstream>

namespace Astra {
    template<typename T>
    void formatHelper(std::ostringstream& oss, std::string_view& str, const T& value) {
        std::size_t openBracket = str.find('{');
        if (openBracket == std::string::npos) { return; }
        std::size_t closeBracket = str.find('}', openBracket + 1);
        if (closeBracket == std::string::npos) { return; }
        oss << str.substr(0, openBracket) << value;
        str = str.substr(closeBracket + 1);
    }

    template<typename... Targs>
    std::string myFormat(std::string_view str, Targs&& ...args) {
        std::ostringstream oss;
        (formatHelper(oss, str, args), ...);
        oss << str;
        return oss.str();
    }
}
