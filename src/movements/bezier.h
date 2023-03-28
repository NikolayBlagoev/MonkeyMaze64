#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <cmath>
#include <sys/time.h>
#include <ctime>
#include <chrono>
class BezierCurve3d{
    public: 
        BezierCurve3d(glm::vec3 p1t, glm::vec3 p2t, glm::vec3 p3t, glm::vec3 p4t) : p1(p1t), p2(p2t), p3(p3t), p4(p4t) {

        };
        std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;
        float total_time = -1;
        glm::vec3 p1, p2, p3, p4;
        glm::vec3 pos_t(float t){
            if(t<0) return p1;
            if(t>1) return p4;
            return float(pow(1.f-t,3))*p1 + 3 * float(pow(1.f-t,2)) *t * p3 + 3 * float(pow(t,2)) * (1.f - t) * p3 + float(pow(t,3))*p4;
        }
};

class BezierCurve4d{
    public: 
        BezierCurve4d(glm::vec4 p1t, glm::vec4 p2t, glm::vec4 p3t, glm::vec4 p4t) : p1(p1t), p2(p2t), p3(p3t), p4(p4t) {

        };
        std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;
        float total_time = -1;
        glm::vec4 p1, p2, p3, p4;
        glm::vec4 pos_t(float t){
            if(t>1) return p4;
            return float(pow(1.f-t,3))*p1 + 3 * float(pow(1.f-t,2)) *t * p3 + 3 * float(pow(t,2)) * (1.f - t) * p3 + float(pow(t,3))*p4;
        }
};