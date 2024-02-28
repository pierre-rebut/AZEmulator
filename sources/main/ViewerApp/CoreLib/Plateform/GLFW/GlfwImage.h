//
// Created by pierr on 10/03/2022.
//

#pragma once

// lib
#include <GLFW/glfw3.h>

// std
#include <string>

namespace Astra::UI::Core::Glfw {

    class GlfwImage
    {
    private:
        unsigned char* m_pixels;
    public:
        GLFWimage image{};

        explicit GlfwImage(const char* pFilepath);
        ~GlfwImage();
    };

}

