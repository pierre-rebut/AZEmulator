//
// Created by pierr on 22/08/2023.
//

#pragma once

#include <exception>
#include <string>
#include "Commons/format.h"

namespace Astra {

    class AstraException : public std::exception
    {
    private:
        std::string m_msg;

    public:
        template<typename... Args>
        explicit AstraException(const std::string_view& pMsg, Args&& ... pArgs) {
            m_msg = myFormat(pMsg, std::forward<Args>(pArgs)...);
        }

        const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override {
            return m_msg.c_str();
        }

        virtual const char* name() const {return "AstraException";}

        template<typename... Args>
        inline static void assertV(bool test, const std::string_view& msg, Args&&... args) {
            AstraException::internalAssertV<AstraException>(test, msg, std::forward<Args>(args)...);
        }

    protected:
        template<class TException = AstraException, typename... Args>
        inline static void internalAssertV(bool test, const std::string_view& pMsg, Args&& ... pArgs) {
            if (!test) {
                throw TException(pMsg, std::forward<Args>(pArgs)...);
            }
        }
    };

} // Astra
