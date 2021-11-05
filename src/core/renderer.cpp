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

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        batch = new SpriteBatch();
    }

    Renderer::~Renderer() {
        delete batch;
    }

    void Renderer::update() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        batch->proj(App::instance->proj);

        auto regist = App::instance->control->regist;
        regist->each([&](const entt::entity e) {
            if(!regist->valid(e)) return;
            if(regist->any_of<SpriteComp>(e)) regist->get<SpriteComp>(e).update();
        });

        batch->flush();
    }
}
