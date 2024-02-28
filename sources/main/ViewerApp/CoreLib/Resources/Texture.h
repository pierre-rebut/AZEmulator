//
// Created by pierr on 16/03/2023.
//
#pragma once

#include <string>
#include <filesystem>

#include <GL/gl.h>
#include "imgui.h"

namespace Astra::UI::Core {

    class Texture
    {
    private:
        GLuint m_textureId = 0;

        int m_width = 0;
        int m_height = 0;
        int m_channels = 0;

        void createImage();

    public:
        explicit Texture(const unsigned char* pData, int pWidth, int pHeight);

        explicit Texture(const std::filesystem::path& pFilename);

        ~Texture();

        inline ImVec2 textureSize() const { return {static_cast<float>(m_width), static_cast<float>(m_height)}; }

        inline ImTextureID textureId() const { return (ImTextureID) m_textureId; }

        void updateTexture(const unsigned char* pData) const;
    };

}
