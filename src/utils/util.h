#ifndef _UTIL_H
#define _UTIL_H

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

struct ObjectTransform {
    glm::vec3 translate;
    glm::vec3 selfRotate; // Angles in degrees
    glm::vec3 rotateParent;
    glm::vec3 scale;
};

#endif //_UTIL_H
