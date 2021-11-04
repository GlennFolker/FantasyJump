#include <SDL.h>
#include <gl/glew.h>

#include "renderer.h"
#include "entity.h"
#include "../app.h"

namespace Fantasy {
    Renderer::Renderer() {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        glDepthFunc(GL_LEQUAL);

        batch = new SpriteBatch();
    }

    Renderer::~Renderer() {
        delete batch;
    }

    void Renderer::update() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        batch->proj(App::instance->proj);

        App::instance->control->regist->each([this](const entt::entity e) {
            if(App::instance->control->regist->any_of<SpriteComp>(e)) App::instance->control->regist->get<SpriteComp>(e).update();
        });

        batch->flush();
    }
}
