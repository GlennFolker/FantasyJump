#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H

#include "mesh.h"
#include "shader.h"

#include <glm/mat4x4.hpp>

constexpr const char *DEFAULT_VERTEX_SHADER = "\
#version 150\n\
in vec3 a_position;\n\
in vec2 a_tex_coords_0;\n\
\n\
uniform mat4 u_proj;\n\
uniform mat4 u_trans;\n\
\n\
out vec2 v_tex_coords;\n\
\n\
void main() {\n\
    gl_Position = u_proj * u_trans * vec4(a_position.x, a_position.y, a_position.z, 1.0);\n\
    v_tex_coords = a_tex_coords_0;\n\
}\n\
";

constexpr const char *DEFAULT_FRAGMENT_SHADER = "\
#version 150\n\
out vec4 fragColor;\n\
\n\
uniform sampler2D u_texture;\n\
in vec2 v_tex_coords;\n\
\n\
void main() {\n\
    fragColor = texture2D(u_texture, v_tex_coords);\n\
}\n\
";

using namespace glm;

namespace Fantasy {
    class SpriteBatch {
        private:
        static Shader *defaultShader;

        bool batching;
        size_t index;

        Mesh *mesh;
        Shader *shader;
        mat4 projection;
        mat4 transform;

        public:
        SpriteBatch();
        SpriteBatch(size_t, Shader *);
        ~SpriteBatch();

        void begin();
        void end();
    };
}

#endif
