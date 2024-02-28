//
// Created by pierr on 10/03/2022.
//

#include "GlfwImage.h"

// lib
#include "stb_image.h"
#include "Commons/AstraException.h"


namespace Astra::UI::Core::Glfw {

    GlfwImage::GlfwImage(const char* pFilepath) {
        int texWidth;
        int texHeight;
        int texChannels;

        m_pixels = stbi_load(pFilepath, &texWidth, &texHeight, &texChannels, STBI_default);
        AstraException::assertV(m_pixels, "Can not load LyioIcon");
        image = {
                texWidth,
                texHeight,
                m_pixels
        };
    }

    GlfwImage::~GlfwImage() {
        stbi_image_free(m_pixels);
    }

}

