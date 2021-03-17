About:
This project is my attempt to make a simple physics engine. the engine utilizes AABB BVH as broadphase and GJK for narrowphase.
it uses EPA to calculate penetration and is applied during the resolution using impulse resolution. the engine utilizes C++ threads
and mutexs to populate data of pairs performing SIMD for both broad phase and narrow phase and collision resolution. 

VERY IMPORTANT: The Imgui Editor is on the top-left corner and it may be collapsed and needs to be expanded.

IN CASE THERE IS NEED TO COMPILE THE PROJECT:
The Source.zip file contains the whole visual studio project, if need be, just unzip and open solution.
run in x86 configuration.

The videos provided show the simulation with different number of objects and the FPS it runs on

Controls:

WASD - Move camera
Right mouse click (hold) - look around
mouse scroll - camera zoom

Key "ESC" - End Simulation
Key "n" - Pause simulation
Key "m" - Resume simulation

Key "o" - Stop Debug Draw
Key "i" - Start Debug Draw
Key "space" - Move camera up
Key "c" - Move camera down

*THE FOLLOWING KEYS NEED TO BE RELEASED IN ORDER TO TRIGGER*

Key "p" - Advance frame when paused
Key "b" - print BVH to console
key "v" - Reconstruct compact tree