#include "texture.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <fmt/format.h>
DISABLE_WARNINGS_POP()
#include <framework/image.h>

#include <iostream>

Texture::Texture(std::filesystem::path filePath) {
    // Load image from disk to CPU memory.
    Image cpuTexture(filePath);

    // Store file name for identification
    fileName = filePath.filename().string();

    // Create texture with correct format, upload data, and generate mipmaps
    glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
    switch (cpuTexture.channels) {
        case 1:
            glTextureStorage2D(m_texture, 1, GL_R8, cpuTexture.width, cpuTexture.height);
            glTextureSubImage2D(m_texture, 0, 0, 0, cpuTexture.width, cpuTexture.height, GL_RED, GL_UNSIGNED_BYTE, cpuTexture.pixels.data());
            break;
        case 3:
            glTextureStorage2D(m_texture, 1, GL_RGB8, cpuTexture.width, cpuTexture.height);
            glTextureSubImage2D(m_texture, 0, 0, 0, cpuTexture.width, cpuTexture.height, GL_RGB, GL_UNSIGNED_BYTE, cpuTexture.pixels.data());
            break;
        case 4:
            glTextureStorage2D(m_texture, 1, GL_RGBA8, cpuTexture.width, cpuTexture.height);
            glTextureSubImage2D(m_texture, 0, 0, 0, cpuTexture.width, cpuTexture.height, GL_RGBA, GL_UNSIGNED_BYTE, cpuTexture.pixels.data());
            break;
        default:
            std::cerr << "Number of channels read for texture is not supported" << std::endl;
            throw std::exception();
    }
    glGenerateTextureMipmap(m_texture);

    // Set behavior for when texture coordinates are outside the [0, 1] range.
    glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Set interpolation for texture sampling (mipmaps)
    glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::Texture(Texture&& other)
    : m_texture(other.m_texture) { other.m_texture = INVALID; }

Texture::~Texture() { if (m_texture != INVALID) { glDeleteTextures(1, &m_texture); } }

void Texture::bind(GLint textureSlot) const {
    glActiveTexture(textureSlot);
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

const Texture& TextureManager::addTexture(std::filesystem::path filePath) {
    textures.push_back(Texture(filePath));
    return textures.back();
}

bool TextureManager::removeTexture(std::string fileName) {
    for (size_t idx = 0UL; idx < textures.size(); idx++) {
        if (fileName == textures[idx].getFileName()) {
            textures.erase(textures.begin() + idx);
            return true;
        }
    }
    return false;
}

const Texture* TextureManager::getTexture(std::string fileName) const {
    for (const Texture& tex : textures) { if (fileName == tex.getFileName()) { return &tex; } }
    return nullptr;
}
