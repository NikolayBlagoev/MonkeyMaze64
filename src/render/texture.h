#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <framework/opengl_includes.h>

#include <exception>
#include <filesystem>
#include <functional>
#include <optional>
#include <memory>
#include <vector>

struct ImageLoadingException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Texture {
    void bind(GLint textureSlot) const;

    static constexpr GLuint INVALID = 0xFFFFFFFF;
    GLuint m_texture { INVALID };
    std::string fileName;
};

class TextureManager {
public:
    std::weak_ptr<const Texture> addTexture(std::filesystem::path filePath);
    bool removeTexture(std::string fileName);
    std::weak_ptr<const Texture> getTexture(std::string fileName) const;

private:
    std::vector<std::shared_ptr<Texture>> textures;
};

#endif
