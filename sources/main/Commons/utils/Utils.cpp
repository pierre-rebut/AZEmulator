//
// Created by pierr on 25/03/2023.
//

#include <sstream>
#include <string>
#include <algorithm>

#include "Utils.h"

namespace Astra::Utils {
    std::vector<std::string> SplitString(const std::string_view& string, const std::string_view& delimiters) {
        size_t first = 0;

        std::vector<std::string> result;

        while (first <= string.size()) {
            const auto second = string.find_first_of(delimiters, first);

            if (first != second) {
                result.emplace_back(string.substr(first, second - first));
            }

            if (second == std::string_view::npos) {
                break;
            }

            first = second + 1;
        }

        return result;
    }

    std::vector<std::string> SplitString(const std::string_view& string, char delimiter) {
        return SplitString(string, std::string(1, delimiter));
    }

    std::string TrimString(const std::string& string) {
        auto startPos = string.find_first_not_of(" \t");

        if (startPos == std::string::npos) {
            return "";
        }

        auto stopPos = string.find_last_not_of(" \t") - startPos + 1;

        return string.substr(startPos, stopPos);
    }

    std::string ToLower(const std::string_view& string) {
        std::string result;
        for (const auto& character: string) {
            result += std::tolower((int) character);
        }

        return result;
    }

    std::string ToUpper(const std::string_view& string) {
        std::string result;
        for (const auto& character: string) {
            result += std::toupper((int) character);
        }

        return result;
    }

    std::string_view RemoveExtension(const std::string_view& filename) {
        return filename.substr(0, filename.find_last_of('.'));
    }

    std::string StrTruncate(const std::string& string, std::size_t width, bool showEllipsis) {
        if (string.length() > width) {
            if (showEllipsis)
                return string.substr(0, width) + "...";
            else
                return string.substr(0, width);
        }
        return string;
    }

    size_t ParseNumber(const std::string& string) {
        return std::stoull(string, nullptr, 0);
    }

    bool StrContains(const std::string_view & strHaystack, const std::string_view & strNeedle) {
        auto it = std::search(
                strHaystack.begin(), strHaystack.end(),
                strNeedle.begin(), strNeedle.end(),
                [](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
        );
        return (it != strHaystack.end());
    }
}
