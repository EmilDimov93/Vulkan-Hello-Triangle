# Hello Triangle

A minimal Vulkan program that renders a triangle.

All code is contained within the Renderer class and can be found in [helloTriangle.cpp](helloTriangle.cpp).

The class has a constructor, a run() method and a destructor called in this order.

The triangle is made up of hardcoded vertices found at the end of the constructor.

## Requirements

- A GPU and driver supporting Vulkan 1.3 with the dynamic rendering feature

- Vulkan SDK (headers, loader, and glslc for shader compilation)

- GLFW

- C++ compiler (C++17 or later)

## Build & Run

A [CMake](CMakeLists.txt) file is included.

I suggest using the [run.ps1](run.ps1) file, which compiles the shaders, builds the project with CMake and runs the ***.exe***.

<img width="245" height="266" alt="image" src="https://github.com/user-attachments/assets/03a64bfc-cbe0-4be0-9165-4b1c1a2da640" />

## Note

Since the point is for the program to be minimal, there are no error checks or validation layers in the code.