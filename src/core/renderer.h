#ifndef RENDERER_H
#define RENDERER_H

#include <box2d/box2d.h>

#include "../app_listener.h"
#include "../graphics/sprite_batch.h"
#include "../graphics/frame_buffer.h"

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
    class Renderer: public AppListener {
        public:
        SpriteBatch *batch;
        FrameBuffer *buffer;
        Mesh *quad;
        Shader *bloom;
        Tex2D *ringTexture;
        Tex2D *pixelTexture;

        public:
        Renderer();
        ~Renderer() override;
        void update() override;
    };
}

#endif
