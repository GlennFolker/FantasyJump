#include <SDL.h>
#include <gl/glew.h>
#include <glm/gtx/transform.hpp>

#include "renderer.h"
#include "assets.h"
#include "../app.h"

namespace Fantasy {
    Tex2D *texture;

    Renderer::Renderer() {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        glDepthFunc(GL_LEQUAL);

        batch = new SpriteBatch();
        position = vec2(0.0f, 0.0f);

        std::function<void(AssetManager *, const char *, Tex2D *)> callback = [](AssetManager *, const char *, Tex2D *data) {
            texture = data;
        };

        App::instance->assets->load<Tex2D>("texture.png", &callback);
        App::instance->assets->finish();
    }

    Renderer::~Renderer() {
        batch->~SpriteBatch();
    }

    void Renderer::update() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float w = App::instance->getWidth() / 2.0f;
        float h = App::instance->getHeight() / 2.0f;
        batch->proj(ortho(
            position.x - w, position.x + w,
            position.y - h, position.y + h
        ));

        batch->draw(texture, 0.0f, 0.0f);
        batch->flush();
    }
}
