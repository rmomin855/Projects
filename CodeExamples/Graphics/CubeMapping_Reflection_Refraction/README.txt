/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: README - information on the assignment
Language: C++
Platform: Working GPU, Windows OS, Visual Studio 2019
Author: Rahil Momin, r.momin, 180001717
Creation date: 06-12-19
*/

RUN UNDER x86 CONFIGURATION

1) There is a drop down menu called "Editor" with 5 tabs.
General tab:
Implementations of Assignment 1,RU
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

Environment tab:
- Use Reflection Button uses reflection as entity for environment mapping
- Use Refraction Button uses refraction entity for environment mapping
- Use Mixed uses Reflection and Refraction and Fresnal's approximation
- Refraction Index Refraction index of the central object
- Interpolation Factor interpolates between phong-model and environment mapping
- FresnelPower power used in Fresnal's approximation

2) Assumptions:
- solution will be run using x86 configuration
- Environment starts with using reflection as entity
- To Draw Vertex/Face Normals or draw path of spheres, Face Normals would be toggled first

3) Parts of the Assignment completed:
- run program without errors
- Assignment 1 parts included and working
- Assignment 2 parts included and working
- Cube mapping
- skybox
- FBO with 6 textures
- environment mapping with Reflection and Refraction
- fresnel's approximation

4) Parts of the Assignment not completed/missing:

5) Relevent Paths where code resides:
- $(SolutionDir)Common/shaders/  (for shaders being used)
BlinnShading.frag
BlinnShading.vert
PhongLighting.frag
PhongLighting.vert
PhongShading.frag
PhongShading.vert
QuadFragmentShader.frag
QuadVertexShader.vert
Simple.frag
Simple.vert
Skybox.frag
Skybox.vert

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
Environment.cpp
Environment.h
Skybox.cpp
Skybox.h

6) Number of hours spent on this Assignment : 8 hours