#include "mesh.h"
#include <framework/disable_all_warnings.h>
#include <framework/mesh.h>
DISABLE_WARNINGS_PUSH()
#include <fmt/format.h>
DISABLE_WARNINGS_POP()
#include <iostream>
#include <vector>

GPUMesh::GPUMesh(Mesh& cpuMesh)
{
    // Create Element(/Index) Buffer Objects and Vertex Buffer Object.
    glCreateBuffers(1, &m_ibo);
    glNamedBufferStorage(m_ibo, static_cast<GLsizeiptr>(cpuMesh.triangles.size() * sizeof(decltype(cpuMesh.triangles)::value_type)), cpuMesh.triangles.data(), 0);
    glCreateBuffers(1, &m_vbo);
    glNamedBufferStorage(m_vbo, static_cast<GLsizeiptr>(cpuMesh.vertices.size() * sizeof(decltype(cpuMesh.vertices)::value_type)), cpuMesh.vertices.data(), 0);

    // Bind vertex data to shader inputs using their index (location).
    // These bindings are stored in the Vertex Array Object.
    glCreateVertexArrays(1, &m_vao);

    // The indices (pointing to vertices) should be read from the index buffer.
    glVertexArrayElementBuffer(m_vao, m_ibo);

    // We bind the vertex buffer to slot 0 of the VAO and tell the VBO how large each vertex is (stride).
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));

    // Tell OpenGL that we will be using vertex attributes [0:4]
    glEnableVertexArrayAttrib(m_vao, 0);
    glEnableVertexArrayAttrib(m_vao, 1);
    glEnableVertexArrayAttrib(m_vao, 2);
    glEnableVertexArrayAttrib(m_vao, 3);
    glEnableVertexArrayAttrib(m_vao, 4);

    // We tell OpenGL what each vertex looks like and how they are mapped to the shader (location = ...).
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, false, offsetof(Vertex, position));
    glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, false, offsetof(Vertex, normal));
    glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, false, offsetof(Vertex, texCoord));
    glVertexArrayAttribFormat(m_vao, 3, 3, GL_FLOAT, false, offsetof(Vertex, tangent));
    glVertexArrayAttribFormat(m_vao, 4, 3, GL_FLOAT, false, offsetof(Vertex, bitangent));

    // For each of the vertex attributes we tell OpenGL to get them from VBO at slot 0.
    glVertexArrayAttribBinding(m_vao, 0, 0);
    glVertexArrayAttribBinding(m_vao, 1, 0);
    glVertexArrayAttribBinding(m_vao, 2, 0);
    glVertexArrayAttribBinding(m_vao, 3, 0);
    glVertexArrayAttribBinding(m_vao, 4, 0);

    // Each triangle has 3 vertices.
    m_numIndices = static_cast<GLsizei>(3 * cpuMesh.triangles.size());
}

GPUMesh::GPUMesh(GPUMesh&& other) { moveInto(std::move(other)); }

GPUMesh::~GPUMesh() { freeGpuMemory(); }

GPUMesh& GPUMesh::operator=(GPUMesh&& other) {
    moveInto(std::move(other));
    return *this;
}

void GPUMesh::draw() const {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr);
}

void GPUMesh::moveInto(GPUMesh&& other) {
    freeGpuMemory();
    m_numIndices    = other.m_numIndices;
    m_ibo           = other.m_ibo;
    m_vbo           = other.m_vbo;
    m_vao           = other.m_vao;
    m_albedo        = other.m_albedo;
    m_normal        = other.m_normal;
    m_metallic      = other.m_metallic;
    m_roughness     = other.m_roughness;
    m_ao            = other.m_ao;

    other.m_numIndices  = 0;
    other.m_ibo         = INVALID;
    other.m_vbo         = INVALID;
    other.m_vao         = INVALID;
    other.m_albedo      = std::weak_ptr<Texture>();
    other.m_normal      = std::weak_ptr<Texture>();
    other.m_metallic    = std::weak_ptr<Texture>();
    other.m_roughness   = std::weak_ptr<Texture>();
    other.m_ao          = std::weak_ptr<Texture>();
}

void GPUMesh::freeGpuMemory() {
    if (m_vao != INVALID) { glDeleteVertexArrays(1, &m_vao); }
    if (m_vbo != INVALID) { glDeleteBuffers(1, &m_vbo); }
    if (m_ibo != INVALID) { glDeleteBuffers(1, &m_ibo); }
}
