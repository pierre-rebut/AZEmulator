//
// Created by pierr on 27/02/2022.
//

#pragma once

#include <unordered_map>
#include <string>
#include <vector>

namespace Astra::Utils {
    template<typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest& ... rest) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };

    std::vector<std::string> SplitString(const std::string_view& string, const std::string_view& delimiters);
    std::vector<std::string> SplitString(const std::string_view& string, char delimiter);
    std::string TrimString(const std::string& string);
    std::string ToLower(const std::string_view& string);
    std::string ToUpper(const std::string_view& string);
    std::string StrTruncate(const std::string& string, std::size_t width, bool showEllipsis = true);
    std::string_view RemoveExtension(const std::string_view& filename);
    size_t ParseNumber(const std::string& string);

    bool StrContains(const std::string_view & str1, const std::string_view & str2);
}