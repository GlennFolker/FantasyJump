cmake_minimum_required(VERSION 3.20.2)

project(Fantasy VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Fantasy
    "src/main.cpp"
    "src/app.cpp"
    "src/graphics/shader.cpp"
    "src/graphics/mesh.cpp"
)

find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(Fantasy opengl32 GLEW::GLEW SDL2::SDL2main SDL2::SDL2)
