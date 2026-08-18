cmake_minimum_required(VERSION 3.20)
project(SciFiOutpost LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(glfw)
FetchContent_Declare(
    glew_src
    URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SOURCE_SUBDIR __download_only__
)
FetchContent_MakeAvailable(glew_src)
add_library(glew_static STATIC
    "${glew_src_SOURCE_DIR}/src/glew.c"
)
target_include_directories(glew_static PUBLIC
    "${glew_src_SOURCE_DIR}/include"
)
target_compile_definitions(glew_static PUBLIC GLEW_STATIC)
if(MSVC)
    target_compile_definitions(glew_static PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
find_package(OpenGL REQUIRED)
add_executable(SciFiOutpost
    src/main.cpp
    src/Shader.cpp
    src/Mesh.cpp
    src/Camera.cpp
    src/GBuffer.cpp
    src/SSAO.cpp
    src/Particles.cpp
    src/PostProcess.cpp
    src/GUI.cpp
)
target_include_directories(SciFiOutpost PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(SciFiOutpost PRIVATE
    OpenGL::GL
    glfw
    glew_static
)
add_custom_command(TARGET SciFiOutpost POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:SciFiOutpost>/shaders"
)
