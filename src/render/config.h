#ifndef _CONFIG_H_
#define _CONFIG_H_

struct RenderConfig {
    // Camera (angles in degrees)
    float verticalFOV { 90.0f };
    float zoomedVerticalFOV { 45.0f };
    bool constrainVertical { false };
};

#endif
