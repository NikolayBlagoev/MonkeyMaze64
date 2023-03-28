#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <cmath>
#include <sys/time.h>
#include <ctime>
#include <vector>
#include <chrono>
class BezierCurve3d{
    public: 
        BezierCurve3d(glm::vec3 p1t, glm::vec3 p2t, glm::vec3 p3t, glm::vec3 p4t, float total_timet ) : p1(p1t), p2(p2t), p3(p3t), p4(p4t), total_time(total_timet) {

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
        BezierCurve4d(glm::vec4 p1t, glm::vec4 p2t, glm::vec4 p3t, glm::vec4 p4t, float total_timet ) : p1(p1t), p2(p2t), p3(p3t), p4(p4t), total_time(total_timet) {

        };
        std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;
        float total_time = -1;
        glm::vec4 p1, p2, p3, p4;
        glm::vec4 pos_t(float t){
            if(t>1) return p4;
            return float(pow(1.f-t,3)) * p1 + 3 * float(pow(1.f-t,2)) * t * p3 + 3 * float(pow(t,2)) * (1.f - t) * p3 + float(pow(t,3))*p4;
        }

        static glm::vec4* qToangl(glm::vec4& inp){
            glm::vec4 in = glm::normalize(inp);
            float angle_rad = acos(in.w) * 2.f;
            float angle_deg = angle_rad * 180.f / 3.141592f;
            float x = in.x / sin(angle_rad/2.f);
            float y = in.y / sin(angle_rad/2.f);
            float z = in.z / sin(angle_rad/2.f);
            return new glm::vec4(x,y,z,angle_deg);
        }
};

class CompositeBezier3d{
    public:
        CompositeBezier3d(std::vector<BezierCurve3d*> curvess, bool toLoop, float completeTime) : loop(toLoop), curves(curvess), total(completeTime) {

        };
        glm::vec3 pos_t(float t){
            if(loop){
                while(t-total > 0){
                    t-=total;
                }
            }
            if(t<0) curves.at(0)->p1;
            if(t>total) curves.at(curves.size()-1)->p4;
            
            for(int i = 0; i < curves.size(); i++){
                if(t-curves.at(i)->total_time<0.f){
                    return(curves.at(i)->pos_t(t/curves.at(i)->total_time));
                }
                t = t - curves.at(i)->total_time;
            }
            return glm::vec3(0.f);
        }
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        bool loop = false;
        std::vector<BezierCurve3d*> curves;
        float total = 0;
};
class CompositeBezier4d{
    public:
        CompositeBezier4d(std::vector<BezierCurve4d*> curvess, bool toLoop, float completeTime) : loop(toLoop), curves(curvess), total(completeTime) {

        };
        glm::vec4 pos_t(float t){
            if(loop){
                while(t-total > 0){
                    t-=total;
                }
            }
            if(t<0) curves.at(0)->p1;
            if(t>total) curves.at(curves.size()-1)->p4;
            
            for(int i = 0; i < curves.size(); i++){
                if(t-curves.at(i)->total_time<0.f){
                    return(curves.at(i)->pos_t(t/curves.at(i)->total_time));
                }
                t = t - curves.at(i)->total_time;
            }
            return glm::vec4(0.f);
        }
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        bool loop = false;
        std::vector<BezierCurve4d*> curves;
        float total = 0;
};