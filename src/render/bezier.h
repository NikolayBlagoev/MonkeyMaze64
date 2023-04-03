#ifndef _BEZIER_H_
#define _BEZIER_H_

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <cmath>
#include <ctime>
#include <vector>
#include <chrono>
#include <time.h>
#include "mesh_tree.h"

class BezierCurve3dGeneral{
    public:
        glm::vec3 pos_t(float t){
            return glm::vec3(0.f);
        };

};
class BezierCurve4dGeneral{
    public:
        glm::vec4 pos_t(float t){
            return glm::vec4(0.f);
        };

};
class BezierCurve3d : public BezierCurve3dGeneral{
    public: 
        BezierCurve3d(glm::vec3 p1t, glm::vec3 p2t, glm::vec3 p3t, glm::vec3 p4t, float total_timet ) : p1(p1t), p2(p2t), p3(p3t), p4(p4t), total_time(total_timet) {

        };
        std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;
        float total_time = -1;
        glm::vec3 p1, p2, p3, p4;
        glm::vec3 pos_t(float t){
            if(t<0) return p1;
            if(t>total_time) return p4;
            t = t/total_time;
            return float(pow(1.f-t,3))*p1 + 3 * float(pow(1.f-t,2)) *t * p3 + 3 * float(pow(t,2)) * (1.f - t) * p3 + float(pow(t,3))*p4;
        }
};

class BezierCurve4d : public BezierCurve4dGeneral{
    public: 
        BezierCurve4d(glm::vec4 p1t, glm::vec4 p2t, glm::vec4 p3t, glm::vec4 p4t, float total_timet ) : p1(p1t), p2(p2t), p3(p3t), p4(p4t), total_time(total_timet) {

        };
        std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;
        float total_time = -1;
        glm::vec4 p1, p2, p3, p4;
        glm::vec4 pos_t(float t){
            if(t<0) return p1;
            if(t>total_time) return p4;
            t = t/total_time;
            return float(pow(1.f-t,3)) * p1 + 3 * float(pow(1.f-t,2)) * t * p3 + 3 * float(pow(t,2)) * (1.f - t) * p3 + float(pow(t,3))*p4;
        }

        static glm::vec4 qToangl(glm::vec4& inp){
            glm::vec4 in = glm::normalize(inp);
            float angle_rad = acos(in.w) * 2.f;
            float angle_deg = angle_rad * 180.f / 3.141592f;
            float x = in.x / sin(angle_rad/2.f);
            float y = in.y / sin(angle_rad/2.f);
            float z = in.z / sin(angle_rad/2.f);
            return glm::vec4(x,y,z,angle_deg);
        }
};

class CompositeBezier3d : public BezierCurve3dGeneral{
    public:
        CompositeBezier3d(std::vector<BezierCurve3d*> curvess, bool toLoop, float completeTime) : loop(toLoop), curves(curvess), total(completeTime) {

        };
        glm::vec3 pos_t(float t){
            if(loop){
                int rem = static_cast<int>(floor(t / total));
                t = t - total*rem;
            }
            if(t<0) curves.at(0)->p1;
            if(t>total) curves.at(curves.size()-1)->p4;
            
            for(int i = 0; i < curves.size(); i++){
                if(t-curves.at(i)->total_time<0.f){
                   
                    return curves.at(i)->pos_t(t);
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
class CompositeBezier4d : public BezierCurve4dGeneral{
    public:
        CompositeBezier4d(std::vector<BezierCurve4d*> curvess, bool toLoop, float completeTime) : loop(toLoop), curves(curvess), total(completeTime) {

        };
        glm::vec4 pos_t(float t){
            if(loop){
                int rem = static_cast<int>(floor(t / total));
                t = t - total*rem;
            }
            if(t<0) curves.at(0)->p1;
            if(t>total) curves.at(curves.size()-1)->p4;
            
            for(int i = 0; i < curves.size(); i++){
                if(t-curves.at(i)->total_time<0.f){
                    
                    return curves.at(i)->pos_t(t);
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
class BezierCombo3d {
    public:
        glm::vec3* to_move;
        std::weak_ptr<MeshTree> obj;
        BezierCurve3d* curve;
        BezierCombo3d(BezierCurve3d* curve,glm::vec3* to_move, std::weak_ptr<MeshTree> obj) : curve(curve), to_move(to_move), obj(obj){

        };
        bool move(float t){
            if(obj.expired()){
                
                free(curve);
                return false;
            }
            glm::vec3 ret = curve->pos_t(t);
            *(to_move) = ret;
            return true;
        };
};
class BezierCombo3dcomp {
    public:
        
        
        CompositeBezier3d* curve;
        glm::vec3* to_move;
        std::weak_ptr<MeshTree> obj;
        BezierCombo3dcomp(CompositeBezier3d* curve, glm::vec3* to_move, std::weak_ptr<MeshTree> obj) : curve(curve), to_move(to_move), obj(obj) {

        };
        bool move(float t){
            if(obj.expired()){
                for(int i = 0; i < curve->curves.size(); i++){
                    BezierCurve3d* tmp = curve->curves.at(i);
                    free(tmp);
                }
                free(curve);
                return false;
            }
            glm::vec3 ret = curve->pos_t(t);
            *(to_move) = ret;
            return true;
        };
};
class BezierCombo4d {
    public:
        glm::vec4* to_move;
        std::weak_ptr<MeshTree> obj;
        int choice = 0;
        BezierCurve4d* curve;
        BezierCombo4d(BezierCurve4d* curve, glm::vec4* to_move, std::weak_ptr<MeshTree> obj) : curve(curve), to_move(to_move), obj(obj){

        };
        bool move(float t){
            if(obj.expired()){
                free(curve);
                return false;
            }
            glm::vec4 ret = curve->pos_t(t);
            *(to_move) = BezierCurve4d::qToangl(ret);
            return true;
        };
};

class BezierCombo4dcomp {
    public:
        glm::vec4* to_move;
        std::weak_ptr<MeshTree> obj;
        CompositeBezier4d* curve;
        BezierCombo4dcomp(CompositeBezier4d* curve, glm::vec4* to_move, std::weak_ptr<MeshTree> obj) : curve(curve), to_move(to_move), obj(obj){

        };
        bool move(float t){
            if(obj.expired()){
                for(int i = 0; i < curve->curves.size(); i++){
                    BezierCurve4d* tmp = curve->curves.at(i);
                    free(tmp);
                }
                free(curve);
                return false;
            }
            glm::vec4 ret = curve->pos_t(t);
            *(to_move) = BezierCurve4d::qToangl(ret);
            return true;
        };
};
class BezierCurveRenderer{
    public:
        std::vector<BezierCombo3d*> curves3d;
        std::vector<BezierCombo4d*> curves4d;
        std::vector<BezierCombo3dcomp*> compcurves3d;
        std::vector<BezierCombo4dcomp*> compcurves4d;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        BezierCurveRenderer(std::chrono::time_point<std::chrono::high_resolution_clock> start_time) : start_time(start_time){

        };
        void do_moves(std::chrono::time_point<std::chrono::high_resolution_clock> curr_time){
            float delta = std::chrono::duration<float>(curr_time - start_time).count();
            
            for(size_t i = 0; i < curves3d.size(); i++){
                if(!curves3d.at(i)->move(delta)){
                    BezierCombo3d* tmp = curves3d.at(i);
                    free(tmp);
                    curves3d.erase(curves3d.begin() + i);
                }
            }
            for(size_t i = 0; i < curves4d.size(); i++){
                if(!curves4d.at(i)->move(delta)){
                    BezierCombo4d* tmp = curves4d.at(i);
                    free(tmp);
                    curves4d.erase(curves4d.begin() + i);
                }
            }
            for(size_t i = 0; i < compcurves3d.size(); i++){
                
                if(!compcurves3d.at(i)->move(delta)){
                    BezierCombo3dcomp* tmp = compcurves3d.at(i);
                    free(tmp);
                    compcurves3d.erase(compcurves3d.begin() + i);
                }
            }
            for(size_t i = 0; i < compcurves4d.size(); i++){
                if(!compcurves4d.at(i)->move(delta)){
                    BezierCombo4dcomp* tmp = compcurves4d.at(i);
                    free(tmp);
                    compcurves4d.erase(compcurves4d.begin() + i);
                }
            }
        };
        size_t add3d(BezierCombo3d* curve3d){
            curves3d.push_back(curve3d);
            return curves3d.size() - 1UL;
        }
        size_t add3dcomp(BezierCombo3dcomp* curve3d){
            compcurves3d.push_back(curve3d);
            return compcurves3d.size() - 1UL;
        }
        size_t add4d(BezierCombo4d* curve4d){
            curves4d.push_back(curve4d);
            return curves4d.size() - 1UL;
        }
        size_t add4dcomp(BezierCombo4dcomp* curve4d){
            compcurves4d.push_back(curve4d);
            return compcurves4d.size() - 1UL;
        }
    
};

#endif
