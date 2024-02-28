//
// Created by pierr on 28/07/2023.
//
#pragma once

#include <filesystem>

namespace Astra {

    class SharedLibrary
    {
    private:
        void* m_lib;

        void* getFunction(const std::string& pName) const;
    public:
        explicit SharedLibrary(const std::filesystem::path& lib);
        ~SharedLibrary();

        template<typename T> requires std::is_function_v<T>
        T* get(const std::string& pName) const {
            return (T*) getFunction(pName);
        }
    };

}
