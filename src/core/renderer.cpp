#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SDL.h>
#include <gl/glew.h>

#include "renderer.h"
#include "entity.h"
#include "time.h"
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

        ringTexture = new Tex2D("assets/ring.png");
        ringTexture->load();
        ringTexture->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);

        pixelTexture = new Tex2D("assets/pixel.png");
        pixelTexture->load();
        pixelTexture->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);

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
        delete ringTexture;
        delete pixelTexture;
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

        float time = Time::time();
        b2World *world = App::instance->control->world;
        entt::registry *regist = App::instance->control->regist;
        entt::entity player = App::instance->control->player;

        if(regist->valid(player)) {
            b2Vec2 pos = regist->get<RigidComp>(player).body->GetTransform().p;
            App::instance->pos = vec2(pos.x, pos.y);
        }

        batch->proj(App::instance->proj);
        batch->col(Color::white);
        batch->tint(Color());

        class: public b2QueryCallback {
            public:
            entt::registry *regist = NULL;

            public:
            bool ReportFixture(b2Fixture *fixture) override {
                if(regist == NULL) return false;

                entt::entity e = (entt::entity)fixture->GetBody()->GetUserData().pointer;
                if(regist->valid(e) && regist->any_of<SpriteComp>(e)) regist->get<SpriteComp>(e).update();

                return true;
            }
        } callback;
        callback.regist = regist;

        float x = App::instance->pos.x, y = App::instance->pos.y, w = App::instance->getWidth() / App::instance->scl.x, h = App::instance->getHeight() / App::instance->scl.y;
        b2AABB bound;
        bound.lowerBound = b2Vec2(x - w, y - h);
        bound.upperBound = b2Vec2(x + w, y + h);

        world->QueryAABB(&callback, bound);

        batch->flush();
        buffer->end();

        bloom->bind();
        glUniform1i(bloom->uniformLoc("u_texture"), buffer->texture->active(0));
        glUniform2f(bloom->uniformLoc("u_resolution"), App::instance->getWidth() / 2.5f, App::instance->getHeight() / 2.5f);
        glUniform1i(bloom->uniformLoc("u_range"), 6);
        glUniform1f(bloom->uniformLoc("u_threshold"), 0.3f);
        glUniform1f(bloom->uniformLoc("u_suppress"), 1.6f);

        quad->render(bloom, GL_TRIANGLES, 0, quad->maxIndices);
    }
}
