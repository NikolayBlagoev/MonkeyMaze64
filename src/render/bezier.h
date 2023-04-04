#ifndef _BEZIER_H_
#define _BEZIER_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
DISABLE_WARNINGS_POP()

#include <render/mesh_tree.h>
#include <array>
#include <cmath>
#include <ctime>
#include <vector>
#include <chrono>
#include <ranges>
#include <time.h>

template<typename Type>
class BezierCurveBase {
public:
    template<typename Type>
    Type positionAtTime(float t) const { return Type { 0.0f }; }
};

template<typename Type>
class BezierCurve : public BezierCurveBase<Type> {
public:
    BezierCurve(Type p1t, Type p2t, Type p3t, Type p4t, float total_timet)
    : controlP0(p1t), controlP1(p2t), controlP2(p3t), controlP3(p4t)
    , totalTime(total_timet) {};
    
    Type positionAtTime(float t);

    float totalTime = -1;
    Type controlP0, controlP1, controlP2, controlP3;
};

template<typename Type>
class BezierComposite : public BezierCurveBase<Type> {
public:
    BezierComposite(const std::vector<BezierCurve<Type>>& curves, bool toLoop, float completeTime)
    : loop(toLoop)
    , m_curves(curves)
    , total(completeTime) {};

    Type positionAtTime(float t);

    float total = 0;
    bool loop   = false;
    std::vector<BezierCurve<Type>> m_curves;
};

template<typename Type>
class BezierCombo {
public:
    BezierCombo(const BezierCurve<Type>& curve, Type* toMove, std::weak_ptr<MeshTree> obj)
    : curve(curve)
    , toMove(toMove)
    , obj(obj) {};
    
    bool positionAtTime(float t);

    BezierCurve<Type> curve;
    Type* toMove;
    std::weak_ptr<MeshTree> obj;
};

template<typename Type>
class BezierComboComposite {
public:
    BezierComboComposite(const BezierComposite<Type>& curve, Type* toMove, std::weak_ptr<MeshTree> obj)
    : curve(curve)
    , toMove(toMove)
    , obj(obj) {};

    bool positionAtTime(float t);

    BezierComposite<Type> curve;
    Type* toMove;
    std::weak_ptr<MeshTree> obj;
};

class BezierCurveManager {
public:
    BezierCurveManager(std::chrono::time_point<std::chrono::high_resolution_clock> startTime) : startTime(startTime) {};
    
    void timeStep(std::chrono::time_point<std::chrono::high_resolution_clock> curr_time);
    
    size_t add3d(const BezierCombo<glm::vec3>& curve3d){
        curves3d.push_back(curve3d);
        return curves3d.size() - 1UL;
    }
    size_t add3dComposite(const BezierComboComposite<glm::vec3>& curve3d){
        compCurves3d.push_back(curve3d);
        return compCurves3d.size() - 1UL;
    }
    size_t add4d(const BezierCombo<glm::vec4>& curve4d){
        curves4d.push_back(curve4d);
        return curves4d.size() - 1UL;
    }
    size_t add4dComposite(const BezierComboComposite<glm::vec4>& curve4d){
        compCurves4d.push_back(curve4d);
        return compCurves4d.size() - 1UL;
    }

    std::vector<BezierCombo<glm::vec3>> curves3d;
    std::vector<BezierCombo<glm::vec4>> curves4d;
    std::vector<BezierComboComposite<glm::vec3>> compCurves3d;
    std::vector<BezierComboComposite<glm::vec4>> compCurves4d;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

#endif
