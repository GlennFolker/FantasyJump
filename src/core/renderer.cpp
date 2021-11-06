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

        press = false;
        lastPressed = -1000.0f;
        progress = 0.0f;
        lastPos = b2Vec2();
        lastAngle = 0.0f;

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

        App::instance->input->attach(SDL_MOUSEBUTTONDOWN, [&](InputContext &ctx) {
            if(ctx.read<char>() != SDL_BUTTON_LEFT || !App::instance->control->regist->valid(App::instance->control->player)) return;
            press = ctx.performed;
            lastPressed = Time::time();
        });
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
        float time = Time::time();
        entt::registry *regist = App::instance->control->regist;
        entt::entity player = App::instance->control->player;

        if(regist->valid(player)) {
            JumpComp &comp = App::instance->control->regist->get<JumpComp>(player);
            progress = fminf((time - lastPressed) / comp.timeout, 1.0f);
            if(!press) progress = fmaxf(1.0f - progress * 5.0f, 0.0f);

            b2Vec2 pos = regist->get<RigidComp>(player).body->GetPosition();
            App::instance->pos = vec2(pos.x, pos.y);

            if(press) {
                double x, y;
                App::instance->unproject(App::instance->getMouseX(), App::instance->getMouseY(), &x, &y);

                lastPos = pos;
                lastAngle = degrees(orientedAngle(normalize(vec2(-1.0f, -1.0f)), normalize(vec2(x, y) + vec2(1.0f, 1.0f) - vec2(pos.x, pos.y))));
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        buffer->begin();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        batch->proj(App::instance->proj);
        batch->col(Color::white);
        batch->tint(Color());

        regist->each([&](const entt::entity e) {
            if(!regist->valid(e)) return;
            if(regist->any_of<SpriteComp>(e)) regist->get<SpriteComp>(e).update();
        });

        batch->flush();
        buffer->end();

        bloom->bind();
        glUniform1i(bloom->uniformLoc("u_texture"), buffer->texture->active(0));
        glUniform2f(bloom->uniformLoc("u_resolution"), App::instance->getWidth() / 2.4f, App::instance->getHeight() / 2.4f);
        glUniform1i(bloom->uniformLoc("u_range"), 7);
        glUniform1f(bloom->uniformLoc("u_threshold"), 0.3f);
        glUniform1f(bloom->uniformLoc("u_suppress"), 1.6f);

        quad->render(bloom, GL_TRIANGLES, 0, quad->maxIndices);
    }
}
