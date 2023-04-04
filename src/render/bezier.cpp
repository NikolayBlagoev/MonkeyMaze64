#include "bezier.h"

#include <utils/misc_utils.hpp>

template<typename Type>
Type BezierCurve<Type>::positionAtTime(float t) {
    if (t < 0)          { return controlP0; }
    if (t > totalTime)  { return controlP3; }
    t /= totalTime;
    return  1.0f *   float(pow(1.0f - t, 3.0f)) * 1.0f          * controlP0 +
            3.0f *   float(pow(1.0f - t, 2.0f)) * t             * controlP1 +
            3.0f *   (1.0f - t)          * float(pow(t, 2.0f))  * controlP2 +
            1.0f *   1.0f                * float(pow(t, 3.0f))  * controlP3;
}

template<typename Type>
Type BezierComposite<Type>::positionAtTime(float t) {
    if (loop) {
        int32_t integerComponent    = static_cast<int32_t>(floor(t / total));
        t                           -= total * integerComponent;
    }
    
    if (t < 0.0f)   { m_curves.front().controlP0; }
    if (t > total)  { m_curves.back().controlP3; }
    
    for (BezierCurve<Type>& curve : m_curves){
        if (t - curve.totalTime < 0.0f) { return curve.positionAtTime(t); }
        t -= curve.totalTime;
    }
    return Type { 0.0f };
}

// Needed for quaternion-to-angles conversion (I REALLY can't be assed to figure out how to re-use the general template)
template<>
bool BezierCombo<glm::vec4>::positionAtTime(float t) {
    if (obj.expired()) { return false; }
    glm::vec4 ret   = curve.positionAtTime(t);
    *toMove         = utils::quaternionToAxisAndDegrees(ret);
    return true;
}

template<typename Type>
bool BezierCombo<Type>::positionAtTime(float t) {
    if (obj.expired()) { return false; }
    Type ret    = curve.positionAtTime(t);
    *toMove     = ret;
    return true;
}

template<>
bool BezierComboComposite<glm::vec4>::positionAtTime(float t) {
    if (obj.expired()) { return false; }
    glm::vec4 ret   = curve.positionAtTime(t);
    *toMove         = utils::quaternionToAxisAndDegrees(ret);
    return true;
}

template<typename Type>
bool BezierComboComposite<Type>::positionAtTime(float t) {
    if (obj.expired()) { return false; }
    Type ret    = curve.positionAtTime(t);
    *toMove     = ret;
    return true;
}

void BezierCurveManager::timeStep(std::chrono::time_point<std::chrono::high_resolution_clock> curr_time) {
    float delta = std::chrono::duration<float>(curr_time - startTime).count();
    std::vector<size_t> deletionIndices;
    
    // 3D curves w/ deletion of curves having free'd meshes
    deletionIndices.clear();
    for (size_t curveIdx = 0UL; curveIdx < curves3d.size(); curveIdx++){
        if (!curves3d.at(curveIdx).positionAtTime(delta)) { deletionIndices.push_back(curveIdx); }
    }
    for (size_t deletionIdx : std::views::reverse(deletionIndices)) { curves3d.erase(curves3d.begin() + deletionIdx); }

    // 4D curves w/ deletion of curves having free'd meshes
    deletionIndices.clear();
    for (size_t curveIdx = 0UL; curveIdx < curves4d.size(); curveIdx++){
        if (!curves4d.at(curveIdx).positionAtTime(delta)) { deletionIndices.push_back(curveIdx); }
    }
    for (size_t deletionIdx : std::views::reverse(deletionIndices)) { curves4d.erase(curves4d.begin() + deletionIdx); }

    // 3D composite curves w/ deletion of curves having free'd meshes
    deletionIndices.clear();
    for (size_t curveIdx = 0UL; curveIdx < compCurves3d.size(); curveIdx++){
        if (!compCurves3d.at(curveIdx).positionAtTime(delta)) { deletionIndices.push_back(curveIdx); }
    }
    for (size_t deletionIdx : std::views::reverse(deletionIndices)) { compCurves3d.erase(compCurves3d.begin() + deletionIdx); }

    // 4D composite curves w/ deletion of curves having free'd meshes
    deletionIndices.clear();
    for (size_t curveIdx = 0UL; curveIdx < compCurves4d.size(); curveIdx++){
        if (!compCurves4d.at(curveIdx).positionAtTime(delta)) { deletionIndices.push_back(curveIdx); }
    }
    for (size_t deletionIdx : std::views::reverse(deletionIndices)) { compCurves4d.erase(compCurves4d.begin() + deletionIdx); }
}
