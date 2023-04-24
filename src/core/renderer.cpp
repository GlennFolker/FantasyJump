#include <SDL.h>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/glu.h>

#include "renderer.h"
#include "entity.h"
#include "time.h"
#include "../app.h"
#include "../util/mathf.h"

static constexpr const char *BLOOM_VERTEX_SHADER = R"(
#version 150 core
in vec2 a_position;
in vec2 a_tex_coords_0;

out vec2 v_tex_coords;

void main() {
    gl_Position = vec4(a_position, 1.0, 1.0);
    v_tex_coords = a_tex_coords_0;
})";

static constexpr const char *BLOOM_FRAGMENT_SHADER = R"(
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

static constexpr const char *PARALLAX_VERTEX_SHADER = R"(
#version 150 core
in vec2 a_position;
in vec2 a_tex_coords_0;

out vec2 v_tex_coords_1;
out vec2 v_tex_coords_2;

uniform vec2 u_resolution;
uniform vec2 u_position;

uniform vec2 u_dimension_1;
uniform float u_intensity_1;
uniform vec2 u_dimension_2;
uniform float u_intensity_2;

void main() {
    gl_Position = vec4(a_position, 1.0, 1.0);
    vec2 pos = vec2(u_position.x, -u_position.y);

    vec2 scale_1 = u_resolution / u_dimension_1;
    v_tex_coords_1 = a_tex_coords_0 * scale_1 - (0.5 * scale_1) + (pos * u_intensity_1) / (u_dimension_1 * 2.0);

    vec2 scale_2 = u_resolution / u_dimension_2;
    v_tex_coords_2 = a_tex_coords_0 * scale_2 - (0.5 * scale_2) + (pos * u_intensity_2) / (u_dimension_2 * 2.0);
})";

static constexpr const char *PARALLAX_FRAGMENT_SHADER = R"(
#version 150 core
out vec4 fragColor;

in vec2 v_tex_coords_1;
in vec2 v_tex_coords_2;

uniform sampler2D u_texture_1;
uniform sampler2D u_texture_2;

void main() {
    vec4 col_1 = texture2D(u_texture_1, v_tex_coords_1);
    vec4 col_2 = texture2D(u_texture_2, v_tex_coords_2);
    fragColor = col_1 * col_1.a + col_2 * col_2.a;
})";

namespace Fantasy {
    Renderer::Renderer() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        atlas = new TexAtlas("assets/sprites/texture.atlas");
        batch = new SpriteBatch();
        buffer = new FrameBuffer(App::instance->getWidth(), App::instance->getHeight());
        toRender = new std::vector<entt::entity>();

        bloom = new Shader(BLOOM_VERTEX_SHADER, BLOOM_FRAGMENT_SHADER);
        quad = new Mesh(4, 6, 2, new VertexAttr[2]{VertexAttr::position2D, VertexAttr::texCoords});
        float quadvert[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };
        unsigned short indices[] = {0, 1, 2, 2, 3, 0};
        quad->setVertices(quadvert, 0, sizeof(quadvert) / sizeof(float));
        quad->setIndices(indices, 0, sizeof(indices) / sizeof(unsigned short));

        parallax = new Shader(PARALLAX_VERTEX_SHADER, PARALLAX_FRAGMENT_SHADER);
        background = new Mesh(4, 6, 2, new VertexAttr[2]{VertexAttr::position2D, VertexAttr::texCoords});
        float backvert[] = {
            -1.0f, -1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 0.0f
        };
        background->setVertices(backvert, 0, sizeof(backvert) / sizeof(float));
        background->setIndices(indices, 0, sizeof(indices) / sizeof(unsigned short));

        std::function<Tex2D *(const char *)> bgLoad = [](const char *name) {
            Tex2D *tex = new Tex2D(name);
            tex->load();
            tex->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
            tex->setWrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);

            return tex;
        };

        backTex1 = bgLoad("assets/background/background-1.png");
        backTex2 = bgLoad("assets/background/background-2.png");

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
        delete background;
        delete parallax;
        delete backTex1;
        delete backTex2;
        delete toRender;
    }

    void Renderer::update() {
        float time = Time::time();
        GameController &control = App::icontrol();
        b2World &world = *control.world;
        entt::registry &regist = *control.regist;
        entt::entity player = control.player;

        if(regist.valid(player)) {
            b2Vec2 pos = regist.get<RigidComp>(player).body->GetTransform().p;
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

        buffer->resize(rw, rh);
        buffer->begin();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if(control.isPlaying()) {
            drawEntities();
        } else {
            TexRegion regions[] = {atlas->get("splash-inst-1"), atlas->get("splash-inst-2"), atlas->get("splash-inst-3"), atlas->get("splash-inst-4"), atlas->get("splash-inst-5")};
            float totalHeight = regions[0].height / 2.0f;
            for(int i = 0; i < 5; i++) totalHeight += regions[i].height / 6.0f;

            float height = 0.0f;
            for(int i = 0; i < 5; i++) {
                const TexRegion &region = regions[i];

                float prog = Mathf::clamp((Time::time() - (control.getStartTime() + 0.2f * i + 0.5f)) / 1.5f);
                batch->col(Color(1.0f, 1.0f, 1.0f, 0.0f).lerp(Color::white, prog));
                batch->draw(region,
                    0.0f, totalHeight / 2.0f + height - (1.0f - powf(1.0f - prog, 3.0f) * 1.5f),
                    region.width / 6.0f, region.height / 6.0f
                );

                height -= region.height / 3.0f;
            }

            batch->col(Color::white);
        }

        if(control.getExitTime() != -1.0f) {
            const TexRegion &region = atlas->get("splash-quit");
            batch->col(Color(1.0f, 1.0f, 1.0f, Mathf::clamp((Time::time() - control.getExitTime()) / 1.0f)));

            glm::dvec2 spos;
            unproject(0.0, 0.0, &spos.x, &spos.y);
            batch->draw(region, spos.x, spos.y, spos.x, spos.y - region.height / 8.0f, region.width / 8.0f, region.height / 8.0f);
            batch->col(Color::white);
        }

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

    void Renderer::drawEntities() {
        b2World &world = App::iworld();
        entt::registry &regist = App::iregistry();
        GameController &control = App::icontrol();

        float w = App::instance->getWidth() / scl.x, h = App::instance->getHeight() / scl.y;

        parallax->bind();
        glUniform2f(parallax->uniformLoc("u_resolution"), w, h);
        glUniform2f(parallax->uniformLoc("u_position"), pos.x, pos.y);

        float bgScaleX = scl.x / 4.0f, bgScaleY = scl.y / 4.0f;
        glUniform1i(parallax->uniformLoc("u_texture_1"), backTex1->active(0));
        glUniform2f(parallax->uniformLoc("u_dimension_1"), backTex1->width / bgScaleX, backTex1->height / bgScaleY);
        glUniform1f(parallax->uniformLoc("u_intensity_1"), 0.27f);
        glUniform1i(parallax->uniformLoc("u_texture_2"), backTex2->active(1));
        glUniform2f(parallax->uniformLoc("u_dimension_2"), backTex2->width / bgScaleX, backTex2->height / bgScaleY);
        glUniform1f(parallax->uniformLoc("u_intensity_2"), 0.1f);

        background->render(parallax, GL_TRIANGLES, 0, background->maxIndices);

        batch->col(Color::white);
        batch->tint(Color());

        b2AABB bound;
        bound.lowerBound = b2Vec2(pos.x - w, pos.y - h);
        bound.upperBound = b2Vec2(pos.x + w, pos.y + h);

        world.QueryAABB(this, bound);
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
            regist.get<DrawComp>(e).update();

            if(regist.any_of<HealthComp>(e)) {
                HealthComp &comp = regist.get<HealthComp>(e);
                if(!comp.showBar || !comp.canHurt()) continue;

                b2Vec2 pos = regist.get<RigidComp>(e).body->GetPosition();

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

        regist.view<IdentifierComp>().each([this](const entt::entity &e, IdentifierComp &comp) {
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

        if(control.getRestartTime() != -1.0f) {
            const TexRegion &region = atlas->get("splash-lose");
            batch->draw(region, pos.x, pos.y - 5.0f, region.width / 8.0f, region.height / 8.0f);
        } else if(control.getWinTime() != -1.0f) {
            const TexRegion &region = atlas->get("splash-win");
            batch->draw(region, pos.x, pos.y - 5.0f, region.width / 8.0f, region.height / 8.0f);
        } else {
            const TexRegion &region = atlas->get("splash-intro");
            batch->col(Color(1.0f, 1.0f, 1.0f, 1.0f - Mathf::clamp((Time::time() - (control.getResetTime() + 2.5f)) / 0.5f)));
            batch->draw(region, pos.x, pos.y - 5.0f, region.width / 8.0f, region.height / 8.0f);
            batch->col(Color::white);
        }

        toRender->clear();
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
            newX == nullptr ? &unusedX : newX, newY == nullptr ? &unusedY : newY, &unusedZ
        );
    }
}
