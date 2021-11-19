#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SDL.h>
#include <gl/glew.h>

#include "renderer.h"
#include "entity.h"
#include "time.h"
#include "../app.h"
#include "../util/mathf.h"

constexpr const char *BLOOM_VERTEX_SHADER = R"(
#version 150 core
in vec2 a_position;
in vec2 a_tex_coords_0;

out vec2 v_tex_coords;

void main() {
    gl_Position = vec4(a_position, 1.0, 1.0);
    v_tex_coords = a_tex_coords_0;
})";

constexpr const char *BLOOM_FRAGMENT_SHADER = R"(
#version 150 core
out vec4 fragColor;
in vec2 v_tex_coords;

uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform int u_range;
uniform float u_threshold;
uniform float u_suppress;

void main() {
    float thres = u_threshold * 3.0;
    float range2 = pow(u_range, 2.0);
    vec2 step = vec2(1.0) / u_resolution;

    vec4 sum = vec4(0.0);
    for(int x = -u_range; x <= u_range; x++) {
        for(int y = -u_range; y <= u_range; y++) {
            float distance = x * x + y * y;
            if(distance > range2) continue;

            vec4 col = texture2D(u_texture, v_tex_coords + vec2(x * step.x, y * step.y));
            if((col.r + col.g + col.b) >= thres) sum += col;
        }
    }

    fragColor = texture2D(u_texture, v_tex_coords) + sum / pow(2 * u_range + 1, 2) / u_suppress;
})";

namespace Fantasy {
    Renderer::Renderer() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        atlas = new TexAtlas("assets/texture.atlas");
        batch = new SpriteBatch();
        buffer = new FrameBuffer(App::instance->getWidth(), App::instance->getHeight());
        toRender = new std::vector<entt::entity>();
        quad = new Mesh(4, 6, 2, new VertexAttr[2]{
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
        delete atlas;
        delete batch;
        delete buffer;
        delete quad;
        delete bloom;
        delete toRender;
    }

    void Renderer::update() {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        buffer->resize(App::instance->getWidth(), App::instance->getHeight());
        buffer->begin();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float time = Time::time();
        b2World *world = App::instance->control->world;
        entt::registry *regist = App::instance->control->regist;
        entt::entity player = App::instance->control->player;

        if(regist->valid(player)) {
            b2Vec2 pos = regist->get<RigidComp>(player).body->GetTransform().p;
            App::instance->pos = glm::vec2(pos.x, pos.y);
        }

        batch->proj(App::instance->proj);
        batch->col(Color::white);
        batch->tint(Color());

        float x = App::instance->pos.x, y = App::instance->pos.y, w = App::instance->getWidth() / App::instance->scl.x, h = App::instance->getHeight() / App::instance->scl.y;
        b2AABB bound;
        bound.lowerBound = b2Vec2(x - w, y - h);
        bound.upperBound = b2Vec2(x + w, y + h);

        world->QueryAABB(this, bound);
        std::sort(toRender->begin(), toRender->end(), [](const entt::entity &a, const entt::entity &b) {
            entt::registry &registry = App::iregistry();
            float za = registry.get<DrawComp>(a).z, zb = registry.get<DrawComp>(b).z;

            if(Mathf::near(za, zb)) {
                return (int)a < (int)b;
            } else {
                return za < zb;
            }
        });

        for(const entt::entity &e : *toRender) regist->get<DrawComp>(e).update();
        toRender->clear();
        batch->flush();
        buffer->end();

        bloom->bind();
        glUniform1i(bloom->uniformLoc("u_texture"), buffer->texture->active(0));
        glUniform2f(bloom->uniformLoc("u_resolution"), App::instance->getWidth() / 2.5f, App::instance->getHeight() / 2.5f);
        glUniform1i(bloom->uniformLoc("u_range"), 4);
        glUniform1f(bloom->uniformLoc("u_threshold"), 0.36f);
        glUniform1f(bloom->uniformLoc("u_suppress"), 1.2f);

        quad->render(bloom, GL_TRIANGLES, 0, quad->maxIndices);
    }

    bool Renderer::ReportFixture(b2Fixture *fixture) {
        entt::registry &registry = App::iregistry();

        entt::entity e = (entt::entity)fixture->GetBody()->GetUserData().pointer;
        if(registry.valid(e) && registry.any_of<DrawComp>(e)) toRender->push_back(e);

        return true;
    }
}
