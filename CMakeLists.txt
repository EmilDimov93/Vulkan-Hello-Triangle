cmake_minimum_required(VERSION 3.20)
project(VulkanRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_executable(HelloTriangle
    helloTriangle.cpp
)

target_include_directories(HelloTriangle PRIVATE
    "C:/Program Files/glfw-3.4.bin.WIN64/include"
)

target_link_libraries(HelloTriangle PRIVATE Vulkan::Vulkan)