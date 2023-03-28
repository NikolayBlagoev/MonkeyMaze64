#pragma once

#include <vector>
#include "render/mesh_tree.h"

class CameraObj{
    public:
    CameraObj(MeshTree* cam, MeshTree* stand, MeshTree* stand1 ) : camera(cam), stand2(stand), root(stand1) {

    }
    MeshTree* camera;
    MeshTree* stand2;
    MeshTree* root;

};