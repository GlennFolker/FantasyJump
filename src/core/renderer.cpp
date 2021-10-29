#include <SDL.h>
#include <gl/glew.h>
#include <glm/gtx/transform.hpp>

#include "renderer.h"
#include "../app.h"
#include "../graphics/tex.h"

namespace Fantasy {
    Tex2D *texture;

    Renderer::Renderer() {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        glDepthFunc(GL_LEQUAL);

        batch = new SpriteBatch();

        texture = new Tex2D("assets/texture.png");
        texture->load();
    }

    void Renderer::update() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float w = App::instance->getWidth() / 2.0f, h = App::instance->getHeight() / 2.0f;
        batch->proj(ortho(-w, w, -h, h));
        
        batch->draw(texture, 0.0f, 0.0f, 64.0f, 64.0f, 0.0f);
        batch->draw(texture, 64.0f, 32.0f, 64.0f, 64.0f, 0.0f);
        batch->draw(texture, 16.0f, 48.0f, 32.0f, 32.0f, 0.0f);
        batch->flush();
    }

    void Renderer::dispose() {
        texture->~Tex2D();
        batch->~SpriteBatch();
    }
}
