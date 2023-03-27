#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/opengl_includes.h>

#include <exception>
#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

struct ImageLoadingException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Texture {
public:
    Texture(std::filesystem::path filePath);
    Texture(const Texture&) = delete;
    Texture(Texture&&);
    ~Texture();

    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) = default;

    void bind(GLint textureSlot) const;
    std::string getFileName() const { return fileName; }

private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;
    GLuint m_texture { INVALID };

    std::string fileName;
};

class TextureManager {
public:
    const Texture& addTexture(std::filesystem::path filePath);
    bool removeTexture(std::string fileName);
    const Texture* getTexture(std::string fileName) const;

private:
    std::vector<Texture> textures;
};

#endif
