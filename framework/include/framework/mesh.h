#pragma once
#include "image.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <optional>
#include <span>
#include <utility>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;		// Texture coordinate
	glm::vec3 tangent;		// Tangent and bitangent for normal map transformation(s)
	glm::vec3 bitangent;	

	[[nodiscard]] constexpr bool operator==(const Vertex&) const noexcept = default;
};

struct Material {
	glm::vec3 kd; // Diffuse color.
	glm::vec3 ks{ 0.0f };
	float shininess{ 1.0f };
	float transparency{ 1.0f };

	// Optional texture that replaces kd; use as follows:
	// 
	// if (material.kdTexture) {
	//   material.kdTexture->getTexel(...);
	// }
	std::optional<Image> kdTexture;
};

struct Mesh {
	// Vertices contain the vertex positions and normals of the mesh.
	std::vector<Vertex> vertices;
	// A triangle contains a triplet of values corresponding to the indices of the 3 vertices in the vertices array.
	std::vector<glm::uvec3> triangles;

	Material material;
};

[[nodiscard]] void computeTangents(Vertex& v0, Vertex& v1, Vertex& v2);
[[nodiscard]] std::vector<Mesh> loadMesh(const std::filesystem::path& file, bool normalize = false);
[[nodiscard]] Mesh mergeMeshes(std::span<const Mesh> meshes);