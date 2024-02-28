//
// Created by pierr on 29/10/2023.
//
#pragma once

#include <exception>
#include <string>

namespace Astra::CPU {

    class AstraRunException : public std::exception
    {
    private:
        std::string m_msg;

    public:
        explicit AstraRunException(const std::string_view& pMsg) : m_msg(pMsg) {}

        const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override {
            return m_msg.c_str();
        }

        static const char* name() { return "AstraRunException"; }
    };

}
