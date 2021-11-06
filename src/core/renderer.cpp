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
        buffer = new FrameBuffer(App::instance->getWidth(), App::instance->getHeight(), true, true);
        quad = new Mesh(4, 6, 2, new VertexAttr[]{
            VertexAttr::position2D,
            VertexAttr::texCoords
        });
        
        float vertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };

        unsigned short indices[] = {0, 1, 2, 2, 3, 0};

        quad->setVertices(vertices, 0, sizeof(vertices) / sizeof(float));
        quad->setIndices(indices, 0, sizeof(indices) / sizeof(unsigned short));

        bloom = new Shader(BLOOM_VERTEX_SHADER, BLOOM_FRAGMENT_SHADER);
    }

    Renderer::~Renderer() {
        delete batch;
        delete buffer;
        delete quad;
        delete bloom;
    }

    void Renderer::update() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        buffer->begin();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        entt::registry *regist = App::instance->control->regist;
        entt::entity player = App::instance->control->player;
        if(regist->valid(player)) {
            b2Vec2 pos = regist->get<RigidComp>(player).body->GetPosition();
            App::instance->pos = vec2(pos.x, pos.y);
        }

        batch->proj(App::instance->proj);
        regist->each([&](const entt::entity e) {
            if(!regist->valid(e)) return;
            if(regist->any_of<SpriteComp>(e)) regist->get<SpriteComp>(e).update();
        });

        batch->flush();
        buffer->end();

        bloom->bind();
        float scale = 2.4f;
        glUniform1i(bloom->uniformLoc("u_texture"), buffer->texture->active(0));
        glUniform2f(bloom->uniformLoc("u_resolution"), App::instance->getWidth() / scale, App::instance->getHeight() / scale);
        glUniform1i(bloom->uniformLoc("u_range"), 7);
        glUniform1f(bloom->uniformLoc("u_threshold"), 0.3f);
        glUniform1f(bloom->uniformLoc("u_suppress"), 1.6f);

        quad->render(bloom, GL_TRIANGLES, 0, quad->maxIndices);
    }
}
