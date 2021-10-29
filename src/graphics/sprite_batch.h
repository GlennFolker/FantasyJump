#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H

#include <glm/mat4x4.hpp>

#include "mesh.h"
#include "shader.h"
#include "tex.h"

constexpr const char *DEFAULT_VERTEX_SHADER = R"(
#version 150 core

in vec3 a_position;
in vec2 a_tex_coords_0;

out vec2 v_tex_coords;

uniform mat4 u_proj;

void main() {
    gl_Position = u_proj * vec4(a_position, 1.0);
    v_tex_coords = a_tex_coords_0;
}
)";

constexpr const char *DEFAULT_FRAGMENT_SHADER = R"(
#version 150 core

out vec4 fragColor;

in vec2 v_tex_coords;

uniform sampler2D u_texture;

void main() {
    fragColor = texture2D(u_texture, v_tex_coords);
}
)";

using namespace glm;

namespace Fantasy {
    class SpriteBatch {
        public:
        float z;

        protected:
        Tex2D *lastTexture;
        size_t index;
        size_t spriteSize;

        Mesh *mesh;
        Shader *shader;

        size_t vertLength;
        float *vertices;
        mat4 projection;

        public:
        SpriteBatch();
        SpriteBatch(size_t, Shader *);
        ~SpriteBatch();

        void draw(Tex2D *, float, float);
        void draw(Tex2D *, float, float, float);
        void draw(Tex2D *, float, float, float, float, float);
        void draw(Tex2D *, float, float, float, float, float, float, float);
        void proj(mat4 projection);
        void flush();
    };
}

#endif