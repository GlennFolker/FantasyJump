#include <SDL.h>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gl/glew.h>
#include <gl/GLU.h>

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
uniform float u_suppress;

void main() {
    float range2 = pow(u_range, 2.0);
    vec2 step = vec2(1.0) / u_resolution;

    vec4 sum = vec4(0.0);
    for(int x = -u_range; x <= u_range; x++) {
        for(int y = -u_range; y <= u_range; y++) {
            float distance = x * x + y * y;
            if(distance > range2) continue;

            sum += texture2D(u_texture, v_tex_coords + vec2(x * step.x, y * step.y));
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
        pos = glm::dvec2(0.0, 0.0);
        scl = glm::dvec2(48.0, 48.0);
        proj = glm::identity<glm::dmat4>();
        flipProj = glm::identity<glm::fmat4>();
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
        float time = Time::time();
        b2World *world = App::instance->control->world;
        entt::registry *regist = App::instance->control->regist;
        entt::entity player = App::instance->control->player;

        if(regist->valid(player)) {
            b2Vec2 pos = regist->get<RigidComp>(player).body->GetTransform().p;
            this->pos = glm::vec2(pos.x, pos.y);
        }

        int rw = App::instance->getWidth(), rh = App::instance->getHeight();
        float w = rw / scl.x, h = rh / scl.y;
        proj = glm::ortho(
            pos.x - w, pos.x + w,
            pos.y - h, pos.y + h
        );
        flipProj = glm::ortho(
            pos.x - w, pos.x + w,
            pos.y + h, pos.y - h
        );

        glViewport(0, 0, rw, rh);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        buffer->resize(App::instance->getWidth(), App::instance->getHeight());
        buffer->begin();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        batch->col(Color::white);
        batch->tint(Color());

        b2AABB bound;
        bound.lowerBound = b2Vec2(pos.x - w, pos.y - h);
        bound.upperBound = b2Vec2(pos.x + w, pos.y + h);

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

        for(const entt::entity &e : *toRender) {
            batch->col(Color::white);
            batch->tint(Color());
            regist->get<DrawComp>(e).update();

            if(regist->any_of<HealthComp>(e)) {
                HealthComp &comp = regist->get<HealthComp>(e);
                if(!comp.showBar || !comp.canHurt()) continue;

                b2Vec2 pos = regist->get<RigidComp>(e).body->GetPosition();
                
                float frac = comp.health / comp.maxHealth;
                const TexRegion &region = atlas->get("white");

                batch->tint(Color::red);
                batch->draw(region, pos.x, pos.y - 1.0f, pos.x - 0.625f, pos.y - 1.075, 1.25f, 0.125f);
                batch->tint(Color::green);

                float bw = (int)((1.25f * frac) / 0.125f) * 0.125f;
                batch->draw(region, pos.x, pos.y - 1.0f, pos.x - 0.625f, pos.y - 1.075f, bw, 0.125f);
            }

            batch->col(Color::white);
            batch->tint(Color());
        }
        
        App::iregistry().view<IdentifierComp>().each([this](const entt::entity &e, IdentifierComp &comp) {
            entt::registry &registry = App::iregistry();
            if(comp.id != "leak" || !registry.any_of<RigidComp>(e)) return;

            b2Vec2 target = registry.get<RigidComp>(e).body->GetPosition();
            b2Vec2 pos = b2Vec2(this->pos.x, this->pos.y);

            b2Vec2 result = target - pos;
            result.Normalize();
            
            float angle = glm::orientedAngle(glm::vec2(1.0f, 0.0f), glm::vec2(result.x, result.y));
            result *= 3.0f;

            batch->col(Color::red);
            batch->draw(atlas->get("white"), pos.x + result.x, pos.y + result.y, 0.5f, 0.125f, angle);
            batch->col(Color::white);
        });

        if(App::icontrol().getRestartTime() != -1.0f) {
            const TexRegion &region = atlas->get("splash-lose");
            batch->draw(region, pos.x, pos.y - 5.0f, region.width / 8.0f, region.height / 8.0f);
        } else if(App::icontrol().getWinTime() != -1.0f) {
            const TexRegion &region = atlas->get("splash-win");
            batch->draw(region, pos.x, pos.y - 5.0f, region.width / 8.0f, region.height / 8.0f);
        } else {
            const TexRegion &region = atlas->get("splash-intro");
            batch->col(Color(1.0f, 1.0f, 1.0f, 1.0f - Mathf::clamp((Time::time() - (App::icontrol().getResetTime() + 2.5f)) / 0.5f)));
            batch->draw(region, pos.x, pos.y - 5.0f, region.width / 8.0f, region.height / 8.0f);
            batch->col(Color::white);
        }

        toRender->clear();

        batch->proj(proj);
        batch->flush();
        buffer->end();

        bloom->bind();
        glUniform1i(bloom->uniformLoc("u_texture"), buffer->texture->active(0));
        glUniform2f(bloom->uniformLoc("u_resolution"), App::instance->getWidth() / 2.5f, App::instance->getHeight() / 2.5f);
        glUniform1i(bloom->uniformLoc("u_range"), 4);
        glUniform1f(bloom->uniformLoc("u_suppress"), 1.2f);

        quad->render(bloom, GL_TRIANGLES, 0, quad->maxIndices);
    }

    bool Renderer::ReportFixture(b2Fixture *fixture) {
        entt::registry &registry = App::iregistry();

        entt::entity e = (entt::entity)fixture->GetBody()->GetUserData().pointer;
        if(registry.valid(e) && registry.any_of<DrawComp>(e)) toRender->push_back(e);

        return true;
    }

    void Renderer::unproject(double x, double y, double *newX, double *newY) {
        double unusedX, unusedY, unusedZ;
        int viewport[4];
        viewport[0] = viewport[1] = 0;
        App::instance->getViewport(&viewport[2], &viewport[3]);

        gluUnProject(
            x, y, 0.0,
            glm::value_ptr(glm::identity<glm::dmat4>()), glm::value_ptr(flipProj), viewport,
            newX == NULL ? &unusedX : newX, newY == NULL ? &unusedY : newY, &unusedZ
        );
    }
}
