//
// Created by pierr on 11/09/2021.
//

#pragma once

#include <memory>

#define BIT(x) (1 << (x))

#define BIND_METHOD(fn) std::bind(& # fn, this, std::placeholders::_1)

namespace Astra {
    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}