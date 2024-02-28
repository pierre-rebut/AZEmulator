//
// Created by pierr on 12/03/2023.
//
#pragma once

#include "Commons/AstraException.h"

namespace Astra {
    template<typename T>
    class Singleton
    {
    public:
        inline static T& Get() {
            AstraException::assertV(m_instance, "Singleton not Init: {}", T::NAME);
            return *m_instance;
        }

        Singleton(const Singleton&) = delete;

        Singleton& operator=(const Singleton) = delete;

        virtual ~Singleton() = default;

        inline static void SetSingleton(T* ptr) {
            T::Singleton::m_instance = ptr;
        }

    protected:
        Singleton() = default;

    private:
        inline static T* m_instance = nullptr;
    };
}
