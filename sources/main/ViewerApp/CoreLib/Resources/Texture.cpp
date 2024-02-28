//
// Created by pierr on 16/03/2023.
//

#include "Texture.h"
#include "stb_image.h"

#include "Commons/AstraException.h"
#include "Commons/Profiling.h"
#include "Commons/Log.h"

namespace Astra::UI::Core {

    Texture::Texture(const unsigned char* pData, int pWidth, int pHeight) : m_width{pWidth}, m_height{pHeight} {
        LOG_TRACE("Texture: create from array");
        ENGINE_PROFILE_FUNCTION();

        createImage();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

        LOG_TRACE("Texture: create end");
    }

    Texture::Texture(const std::filesystem::path& pFilename) {
        LOG_TRACE("Texture: create {}", pFilename);
        ENGINE_PROFILE_FUNCTION();

        m_channels = 4;
        unsigned char* imageData = stbi_load(pFilename.string().c_str(), &m_width, &m_height, nullptr, m_channels);
        AstraException::assertV(imageData, "Texture: invalid texture file {}", pFilename);

        createImage();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

        stbi_image_free(imageData);

        LOG_TRACE("Texture: create end");
    }

    Texture::~Texture() {
        LOG_TRACE("Texture: destroy");
        ENGINE_PROFILE_FUNCTION();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &m_textureId);

        LOG_TRACE("Texture: destroy end");
    }

    void Texture::createImage() {
        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);

        // Setup filtering functionsDetails for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    }

    void Texture::updateTexture(const unsigned char* pData) const {
        ENGINE_PROFILE_FUNCTION();

        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, pData);
    }
}
