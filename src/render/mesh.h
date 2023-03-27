#ifndef _MESH_H_
#define _MESH_H_

#include <framework/opengl_includes.h>

#include <render/texture.h>
#include <exception>
#include <filesystem>
#include <memory>

struct MeshLoadingException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class GPUMesh {
public:
    GPUMesh(std::filesystem::path filePath);
    GPUMesh(const GPUMesh&) = delete; // Cannot copy a GPU mesh because it would require reference counting of GPU resources.
    GPUMesh(GPUMesh&&);
    ~GPUMesh();

    // Cannot copy a GPU mesh because it would require reference counting of GPU resources.
    GPUMesh& operator=(const GPUMesh&) = delete;
    GPUMesh& operator=(GPUMesh&&);

    // Bind VAO and call glDrawElements.
    void draw() const;

    // Getters and setters for textures
    std::weak_ptr<const Texture> getAlbedo() const                { return m_albedo; }
    std::weak_ptr<const Texture> getNormal() const                { return m_normal; }
    std::weak_ptr<const Texture> getMetallic() const              { return m_metallic; }
    std::weak_ptr<const Texture> getRoughness() const             { return m_roughness; }
    std::weak_ptr<const Texture> getAO() const                    { return m_ao; }
    void setAlbedo(std::weak_ptr<const Texture> newALbedo)        { m_albedo    = newALbedo; }
    void setNormal(std::weak_ptr<const Texture> newNormal)        { m_normal    = newNormal; }
    void setMetallic(std::weak_ptr<const Texture> newMetallic)    { m_metallic  = newMetallic; }
    void setRoughness(std::weak_ptr<const Texture> newRoughness)  { m_roughness = newRoughness; }
    void setAO(std::weak_ptr<const Texture> newAO)                { m_ao        = newAO; }

private:
    void moveInto(GPUMesh&&);
    void freeGpuMemory();

private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;

    // Indices, vertices, and VAO
    GLsizei m_numIndices { 0 };
    GLuint m_ibo { INVALID };
    GLuint m_vbo { INVALID };
    GLuint m_vao { INVALID };

    // Texture data
    std::weak_ptr<const Texture>  m_albedo       { std::weak_ptr<Texture>() };
    std::weak_ptr<const Texture>  m_normal       { std::weak_ptr<Texture>() };
    std::weak_ptr<const Texture>  m_metallic     { std::weak_ptr<Texture>() };
    std::weak_ptr<const Texture>  m_roughness    { std::weak_ptr<Texture>() };
    std::weak_ptr<const Texture>  m_ao           { std::weak_ptr<Texture>() };
};

#endif
