#ifndef _CONFIG_H_
#define _CONFIG_H_

struct RenderConfig {
    // Camera (angles in degrees)
    float moveSpeed         { 0.03f };
    float lookSpeed         { 0.0015f };
    float verticalFOV       { 60.0f };
    float zoomedVerticalFOV { 35.0f };
    bool constrainVertical  { false };

    // Lighting debug
    bool drawLights { false };
    bool drawSelectedPointLight { false };
};

#endif
