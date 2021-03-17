/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: README - information on the assignment
Language: C++
Platform: Working GPU, Windows OS, Visual Studio 2019
Author: Rahil Momin, r.momin, 180001717
Creation date: 07-03-20
*/

RUN UNDER x86 CONFIGURATION

1) There is a drop down menu called "Editor" with 2 tabs.
General tab:
- Camera Speed is the speed at which the camera is able to traverse the scene
- Camera near/far are the clipping extends, how far can the camera see
- There is a checkbox by the label of "Copy Depth Buffer"
 - CheckBox indicates whether Depth buffer is being copied to the default buffer.
- There are four buttons as follows
 - Button FSQ Target: None, The shader applies the phong model using the g-buffer
 - Button FSQ Target: Position, Displays deferred objects in according to their position buffer of the g-buffer
 - Button FSQ Target: Normal, Displays deferred objects in according to their normal buffer of the g-buffer
 - Button FSQ Target: UV, (currently no UV's being calculated) Displays deferred objects in according to their uv buffer of the g-buffer
- DrawBVH
 - draws the bounding volumes of each level except the leaf bounding volumes
- Draw Object Volumes
 - draws the bounding volumes of all objects, (leaf nodes of BVH)
- The text in GREEN indicates what type of tree is drawn and the bounding volume being used.
 - OBB volume, leaf nodes are OBB but other nodes are AABB
 - Ellipsoid volume, leaf nodes are Ellipsoid but other nodes are spheres
- Ability to change what kind of BVH to use (top down / bottom up)
- Ability to choose the CutOff method for Top-Down (level = 7 or vertices <= 500)
- Ability to change what kind of volume being used (AABB, Sphere, OBB, Ellipsoid)
- Ability to choose between the method used to calculate Bounding Sphere
 - Methods avaliable, Centroid, Ritter's, Larson's, PCA

Lights/Shaders tab:
- Reload Shader button reloads shaders
- 3 fields for Attunation constants used for lights

- emissive colour is used by light (0 - 255)
- fog colour is used by light (0 - 255)
- ambient colour is used by light (0 - 255)
- specular colour is used by light (0 - 255)
- diffuse colour is used by light (0 - 255)

- Ambient field is used as ka (ambient strength) for all lights
- Diffuse field is used as kd Diffuse strength) for all lights
- Specular field is used as ks (Specular strength) for all lights
- Shininess field is used as ns (shininess) for all lights

Camera Controls:
- WASD to move the camera
- space key to go up
- 'c' key to go down
- right-mouse button to look around

2) Assumptions:
- solution will be run using x86 configuration in Release mode
- Camera is a point light
- Top Down tree allows slicing of meshes
- Sphere mesh can be low poly, (compact spheres may make it look like part of mesh is outside)
- Scene to use is PowerPlant4 (if need to change, extract .tar file and place the PPsections in Common/models/PowerPlant folder)
- the vertex cut-off method for top-down means stop when vertices count <= 500

3) Parts of the Assignment completed:
- run program without errors
- Load Scene (PowerPlant4)
- Camera is the source of light
- Bounding Volumes - AABB, Sphere, OBB, Ellipsoid
- Bounding Volume Hierarchy- Top-Down, Bottom-Up - both in AABB/Sphere
- Toggle/Change Bounding Volumes
- Display Bounding Volumes
- First-person camera
- Light incorporated

4) Parts of the Assignment not completed/missing:

5) Relevent Paths where code resides:
- $(SolutionDir)Common/shaders/  (for shaders being used)
QuadFragmentShader.frag
QuadVertexShader.vert
Simple.frag
Simple.vert
Deferred.frag
Deferred.vert
DeferredQuad.frag
DeferredQuad.vert

- $(SolutionDir)Common/textures/  (for textures being used)

- $(SolutionDir)Source/   (for all cpp/header files i've made)
Editor.cpp
Editor.h
MeshManager.cpp
MeshManager.h
Object.cpp
Object.h
ObjectManager.cpp
ObjectManager.h
Transform.cpp
Transform.h
SimpleScene_Quad.cpp
SimpleScene_Quad.h
Light.h
Deffered.cpp
Deffered.h
BVHTree.cpp
BVHTree.h
Camera.cpp
Camera.h
Node.cpp
Node.h

6) Number of hours spent on this Assignment : ~3-4 hours