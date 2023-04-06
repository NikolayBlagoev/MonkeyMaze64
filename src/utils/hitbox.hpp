#ifndef _HITBOX_HPP_
#define _HITBOX_HPP_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()

#include <array>


struct HitBox {
    bool allowCollision;
    std::array<glm::vec3, 8> points;

    glm::vec3 getMiddle() {
        // Assume corner points are first and last
        return {(points[0].x + points[7].x) / 2,
                (points[0].y + points[7].y) / 2,
                (points[0].z + points[7].z) / 2};
    }

    bool collides(const HitBox& other) {
        for (glm::vec3 newPoint : this->points) {
            glm::vec3 smallerOrEqual(0.0f);
            glm::vec3 bigger(0.0f);

            for (glm::vec3 curPoint: other.points) {
                if (newPoint.x <= curPoint.x)   { smallerOrEqual.x++; }
                else                            { bigger.x++; }
                if (newPoint.y <= curPoint.y)   { smallerOrEqual.y++; }
                else                            { bigger.y++; }
                if (newPoint.z <= curPoint.z)   { smallerOrEqual.z++; }
                else                            { bigger.z++; }
            }

            bool collision = (smallerOrEqual.x != 0 && bigger.x != 0) &&
                             (smallerOrEqual.y != 0 && bigger.y != 0) &&
                             (smallerOrEqual.z != 0 && bigger.z != 0);
            if (collision) { return true; }
        }
        return false;
    }

    static HitBox makeHitBox(const Mesh& cpuMesh, bool allowCollision) {
        // Finding minimum and maximum coordinates of X, Y, and Z axes
        std::array<std::array<float, 3>, 2> minMax{};
        for (Vertex vertex : cpuMesh.vertices) {
            if (vertex.position.x < minMax[0][0]) { minMax[0][0] = vertex.position.x; }
            if (vertex.position.x > minMax[1][0]) { minMax[1][0] = vertex.position.x; }
            if (vertex.position.y < minMax[0][1]) { minMax[0][1] = vertex.position.y; }
            if (vertex.position.y > minMax[1][1]) { minMax[1][1] = vertex.position.y; }
            if (vertex.position.z < minMax[0][2]) { minMax[0][2] = vertex.position.z; }
            if (vertex.position.z > minMax[1][2]) { minMax[1][2] = vertex.position.z; }
        }

        // Create bounding box from extreme points
        std::array<glm::vec3, 8> points = {};
        for (int z = 0; z < 2; ++z) {
            for (int y = 0; y < 2; ++y) {
                for (int x = 0; x < 2; ++x) {
                    points[x + y * 2 + z * 4] = glm::vec3(minMax[x][0], minMax[y][1], minMax[z][2]);
                }
            }
        }

        return {allowCollision, points};
    }
};

#endif //_HITBOX_HPP_
