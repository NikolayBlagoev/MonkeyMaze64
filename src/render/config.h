#ifndef _CONFIG_H_
#define _CONFIG_H_

struct RenderConfig {
    // Camera (angles in degrees)
    float moveSpeed         { 0.03f };
    float lookSpeed         { 0.0015f };
    float verticalFOV       { 90.0f };
    float zoomedVerticalFOV { 45.0f };
    bool constrainVertical  { false };
};

#endif
