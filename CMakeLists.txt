cmake_minimum_required(VERSION 3.20.2)

project(Fantasy VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Fantasy
    "src/main.cpp"
    "src/app.cpp"
    "src/graphics/camera.cpp"
    "src/graphics/shader.cpp"
    "src/graphics/mesh.cpp"
    "src/graphics/sprite_batch.cpp"
    "src/core/renderer.cpp"
)

find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)
target_link_libraries(Fantasy opengl32 GLEW::GLEW glm::glm SDL2::SDL2_image SDL2::SDL2main SDL2::SDL2)
