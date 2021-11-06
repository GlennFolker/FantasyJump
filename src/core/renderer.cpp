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

        bloom = new Shader(R"(
#version 150 core
in vec2 a_position;
in vec2 a_tex_coords_0;

out vec2 v_tex_coords;

void main() {
    gl_Position = vec4(a_position, 1.0, 1.0);
    v_tex_coords = a_tex_coords_0;
}
        )", R"(
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
}
        )");
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

        batch->proj(App::instance->proj);

        entt::registry *regist = App::instance->control->regist;
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
