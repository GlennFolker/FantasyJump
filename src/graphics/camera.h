#ifndef CAMERA_H
#define CAMERA_H

#include <glm/mat4x4.hpp>

using namespace glm;

namespace Fantasy {
    struct Camera {
        public:
        mat4 view;
        mat4 projection;
        mat4 combined;
        mat4 invCombined;
        vec3 position;
        vec3 direction;
        vec3 up;

        float width;
        float height;
        float near;
        float far;

        bool isPerspective;
        float fov;

        public:
        Camera();
        void resize(float, float);
        void update();
    };
}

#endif
