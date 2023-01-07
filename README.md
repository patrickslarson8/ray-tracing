# Desktop Ray Tracer

![](2022-12-29-14-58-10.png)

Program to practice with desktop graphics and study 3d math. Currently renders spheres with a fixed lighting source. The spheres attributes are changeable in real-time and the camera can also move.

## Getting Started

1. To use, first download and install the Vulkan SDK: https://vulkan.lunarg.com/
2. Clone the repo
3. Run setup.bat located in Scripts
4. Compile and run the program

### Performance

Current render time with built-in multithreading (for loop) on my computer is 48-50ms per frame. This spawns way too many threads and performance in lost in overhead. Additionally, significant time is spent coming up with randoms for the scene accumulation (see screen shot below)

![](2023-01-07-11-24-14.png)

By pre-calculating the random vectors on initialization, render time is decreased to 7-8ms per frame;

### To Do

1. Improve performance
   1. Change random to predictive directions for roughness calculation
   2. Change multi-threading scheme to use groups aware of how many cores CPU has
2. Move rendering from CPU to GPU
3. Add ability to render objects other than spheres
