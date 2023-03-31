#ifndef _HITBOX_H
#define _HITBOX_H

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <array>

DISABLE_WARNINGS_POP()

struct HitBox {
    bool allowCollision;
    std::array<glm::vec3, 8> points;

    glm::vec3 getMiddle() {
        // assume corner points are first and last
        return {(points[0].x + points[7].x) / 2,
                (points[0].y + points[7].y) / 2,
                (points[0].z + points[7].z) / 2};
    }

    bool collides(HitBox other) {

        for (glm::vec3 newPoint : this.points) {

            glm::vec3 smallerOrEqual(0);
            glm::vec3 bigger(0);

            for (glm::vec3 curPoint: other.points) {
                if (newPoint.x <= curPoint.x)
                    smallerOrEqual.x++;
                else
                    bigger.x++;

                if (newPoint.y <= curPoint.y)
                    smallerOrEqual.y++;
                else
                    bigger.y++;

                if (newPoint.z <= curPoint.z)
                    smallerOrEqual.z++;
                else
                    bigger.z++;
            }

            bool collision = (smallerOrEqual.x != 0 && bigger.x != 0)
                             && (smallerOrEqual.y != 0 && bigger.y != 0)
                             && (smallerOrEqual.z != 0 && bigger.z != 0);

            if (collision)
                return true;
        }

        return false;
    }
};

#endif //_HITBOX_H
