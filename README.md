# MonkeyMaze64
A procedurally generated maze traversal game made with raw C++ and OpenGL. The goal of the game is to collect all 7 Monkey Emeralds in order to turn into a Hyper Monkey (SEGA copyright violations notwithstanding). You need to also avoid being detected by the cameras monitoring the maze.

This was developed for the final project of the course IN4152 3D Computer Graphics and Animation.

## Group Members
- Nikolay Blagoev
- Tobias van den Hurk
- William Narchi

## Compilation Instructions
As this project uses CMake, simply use your favourite CLI/IDE/code editor and compile the `FinalExecutable` target. All dependencies are included in this repository and compiled as needed. The textures and models utilised can be downloaded [here](https://drive.google.com/file/d/1K86r1SvZYmcDIqXRWt8A36VvDofZLhWQ/view?usp=sharing) (Google Drive link) and should be placed in the `resources` folder such that the `models` and `textures` folders are directly inside `resources`, i.e.
```
├── resources
│   ├── models
│   ├── textures
```

## Feature List
- Gameplay
  - Procedural maze generation using rules for matching maze blocks
  - Procedural object placement
  - Minimap via rendering from top-down viewpoint
- Animation and modelling
  - Hierarchical modelling and animation
  - (Composite) Bezier curves for animation
- Rendering
  - Deferred shading
  - PBR shading using Epic/Disney metallic workflow
  - HDR rendering with (exposure) tonemapping and gamma correction
  - Shadow-casting lights
    - Point lights
    - Area lights with adjustable quadratic area
  - Normal mapping
  - Parallax occlusion mapping
  - Transparent particles (depth-sorted and forward rendered on top of deferred shading results)
  - Post-processing effects
    - Bloom
    - Screen-Space Ambient Occlusion
  - Animated textures (by swapping textures on a per-frame basis)
