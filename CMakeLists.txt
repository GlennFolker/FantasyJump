cmake_minimum_required(VERSION 3.14.0)

project(Fantasy VERSION 1.0)

add_executable(Fantasy
    "src/main.cpp"
    "src/app.cpp"
    "src/core/entity.cpp"
    "src/core/game_controller.cpp"
    "src/core/renderer.cpp"
    "src/core/input.cpp"
    "src/core/time.cpp"
    "src/core/content.cpp"
    "src/core/events.cpp"
    "src/core/team.cpp"
    "src/graphics/color.cpp"
    "src/graphics/mesh.cpp"
    "src/graphics/sprite_batch.cpp"
    "src/graphics/shader.cpp"
    "src/graphics/frame_buffer.cpp"
    "src/graphics/tex.cpp"
    "src/util/mathf.cpp"
)

target_compile_features(Fantasy PRIVATE cxx_std_20)

find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2-image CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(Fantasy PRIVATE
        mingw32
        EnTT::EnTT box2d::box2d
        opengl32 GLEW::GLEW glm::glm
        SDL2::SDL2_image SDL2::SDL2main SDL2::SDL2
    )
else()
    target_link_libraries(Fantasy PRIVATE
        EnTT::EnTT box2d::box2d
        opengl32 GLEW::GLEW glm::glm
        SDL2::SDL2_image SDL2::SDL2main SDL2::SDL2
    )
endif()

set(RESOURCE_FILES ${CMAKE_SOURCE_DIR}/assets/)
set(Fantasy PROPERTIES RESOURCE ${RESOURCE_FILES})

add_custom_command(
    TARGET Fantasy POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E remove_directory
    $<TARGET_FILE_DIR:Fantasy>/assets

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:Fantasy>/assets
)

install(TARGETS Fantasy
    ARCHIVE DESTINATION .
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
)
install(DIRECTORY $<TARGET_FILE_DIR:Fantasy>/assets DESTINATION .)

include(InstallRequiredSystemLibraries)
