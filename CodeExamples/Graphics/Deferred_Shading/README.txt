/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: README - information on the assignment
Language: C++
Platform: Working GPU, Windows OS, Visual Studio 2019
Author: Rahil Momin, r.momin, 180001717
Creation date: 31-01-20
*/

RUN UNDER x86 CONFIGURATION

1) There is a drop down menu called "Editor" with 4 tabs.
General tab:
- There is a checkbox by the label of "Copy Depth Buffer"
 - CheckBox indicates whether Depth buffer is being copied to the default buffer.
- There are four buttons as follows
 - Button FSQ Target: None, The shader applies the phong model using the g-buffer
 - Button FSQ Target: Position, Displays deferred objects in according to their position buffer of the g-buffer
 - Button FSQ Target: Normal, Displays deferred objects in according to their normal buffer of the g-buffer
 - Button FSQ Target: UV, Displays deferred objects in according to their uv buffer of the g-buffer
Implementations of CS300,
 - Option to Draw Vertex normal, Fragment normal, draw the path of the spheres
 - In order to turn on debug draw start with Face Normal
 - Change Draw type(fill, point, line)
- able to toggle rotation of sphere ring
- able to toggle rotation of the center object
- able to change "near" and "far", changing this would recalculate the matrix
and the same values are used inside the shaders for phong model

Lights/Shaders tab:
- Reload Shader button reloads shaders
- Phong Shading button uses PhongShading technique for lights
- Phong Lighting button uses PhongLighting technique for lights
- Blinn Shading button uses BlinnShading technique for lights

- Number of lights control the number of active lights (1 - 16)
- Field of view is the outer angle for all spot lights (1 - 45) in degrees
- Inner Angle is the inner angle for all spot lights (0 - outer angle) in degrees
- Fall off controls fall off value for all spot lights

- Point Light button changes all lights to point light
- directional Light button changes all lights to directional light
- Spot Light button changes all lights to spot light
- 3 fields for Attunation constants used for all lights

- emissive colour is used by all lights (0 - 255)
- fog colour is used by all lights (0 - 255)
- ambient colour is used by all lights (0 - 255)
- specular colour is used by all lights (0 - 255)

- Ambient field is used as ka (ambient strength) for all lights
- Diffuse field is used as kd Diffuse strength) for all lights
- Specular field is used as ks (Specular strength) for all lights
- Shininess field is used as ns (shininess) for all lights
- Distance is the distance of spheres from the central object

There are 4 buttons to control the scene
 - Scene1 = all lights have the same colour and light type
 - Scene3 = mixed light colour and type
 there are 2 Scene2 due to confusion in the assignment handout
  - first one has the same colour but different type
  - second one has the same type but different colour

Per Light:
- Light Index = index of sphere to select from active lights (0 - active no. of lights)
- diffuse, ambient, specular change the colours used by selected light (0 - 1)
- Ambient (ka), Diffuse (kd), Specular (ks), shininess (ns) used by selected light
- Half angle is the outer angle for that light, if it is a spot light (0 - 45) degrees
- inner angle is the inner angle used by the light, if it is a spot light (0 - outer angle)
- Fall off is the fall of value for that light, if it is a spot light
- Light Type
 - 0 = Point light
 - 1 = directional light
 - 2 = spot light

Texture tab:
- Toggle Textured turns on/off textures
- cylindrical mapping button uses cylindrical mapping
- spherical mapping button uses spherical mapping
- Planar mapping button uses cube mapping
- Texture Entity : position button changes texture entity to use position
- Texture Entity : normal button changes texture entity to use normal
- Use CPU Generated UV checkBox changes from Pre-calculated uv on CPU to calculating on GPU in runtime

2) Assumptions:
- solution will be run using x86 configuration
- when using textures, both diffuse and specular are to be used at the same time
- Texture starts with cylindrical mapping using position as texture entity

3) Parts of the Assignment completed:
- run program without errors
- CS300 parts included and working
- Load Objects from .obj files
- G-buffer populated
- Lighting pass on the FSQ
- Lighting pass with phong model
- Render light spheres using forward rendering
- Camera Movement WASD
- GUI options to toggle depth buffer
- Ability to visualize individual render targets

4) Parts of the Assignment not completed/missing:

5) Relevent Paths where code resides:
- $(SolutionDir)Common/shaders/  (for shaders being used)
PhongShading.frag
PhongShading.vert
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

6) Number of hours spent on this Assignment : OVER 9000 (not really its <1 hour)