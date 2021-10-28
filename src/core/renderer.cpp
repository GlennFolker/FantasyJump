#include <SDL.h>
#include <gl/glew.h>
#include <glm/gtx/transform.hpp>

#include "renderer.h"
#include "../app.h"

namespace Fantasy {
    Renderer::Renderer() {
        camera = Camera();
        camera.isPerspective = false;
        camera.position = vec3(0.0f, 0.0f, 0.0f);
        camera.direction = vec3(0.0f, 0.0f, 1.0f);
        camera.near = -100.0f;

        batch = new SpriteBatch();
    }

    Renderer::~Renderer() {
        batch->~SpriteBatch();
    }

    void Renderer::update() {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.resize(App::instance->getWidth(), App::instance->getHeight());
        camera.update();
        
        batch->flush(camera.combined);
    }
}
