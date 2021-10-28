#include "camera.h"

#include <glm/gtx/transform.hpp>

namespace Fantasy {
    Camera::Camera() {
        isPerspective = false;
        view = identity<mat4>();
        projection = identity<mat4>();
        combined = identity<mat4>();
        invCombined = identity<mat4>();
        position = vec3(0.0f, 0.0f, -5.0f);
        direction = vec3(0.0f, 0.0f, 0.0f);
        up = vec3(0.0f, 1.0f, 0.0f);

        fov = 67.0f;
        width = 0.0f;
        height = 0.0f;
        near = 1.0f;
        far = 100.0;
    }

    void Camera::resize(float width, float height) {
        this->width = width;
        this->height = height;
    }

    void Camera::update() {
        if(!isPerspective) {
            projection = ortho(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, near, far);
        } else {
            projection = perspective(fov, width / height, near, far);
        }

        view = lookAt(position, position + direction, up);
        combined = projection * view;
        invCombined = inverse(combined);
    }
}
