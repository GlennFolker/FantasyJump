cmake_minimum_required(VERSION 3.20.2)

project(Fantasy VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Fantasy
    "src/main.cpp"
    "src/app.cpp"
    "src/core/assets.cpp"
    "src/core/entity.cpp"
    "src/core/game_controller.cpp"
    "src/core/renderer.cpp"
    "src/core/input.cpp"
    "src/core/time.cpp"
    "src/core/content.cpp"
    "src/assets/tex_loader.cpp"
    "src/graphics/color.cpp"
    "src/graphics/shader.cpp"
    "src/graphics/mesh.cpp"
    "src/graphics/sprite_batch.cpp"
    "src/graphics/tex.cpp"
    "src/util/mathf.cpp"
)

find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)

target_link_libraries(Fantasy
    EnTT::EnTT box2d::box2d
    opengl32 GLEW::GLEW glm::glm
    SDL2::SDL2_image SDL2::SDL2main SDL2::SDL2
)

add_custom_command(
    TARGET Fantasy POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E remove_directory
    $<TARGET_FILE_DIR:Fantasy>/assets

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:Fantasy>/assets
)
