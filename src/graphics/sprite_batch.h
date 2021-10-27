#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H

#include "mesh.h"
#include "shader.h"

#include <glm/mat4x4.hpp>

constexpr const char *DEFAULT_VERTEX_SHADER = "\
#version 150 core\n\
in vec3 a_position;\n\
\n\
uniform mat4 u_proj;\n\
uniform mat4 u_trans;\n\
\n\
void main() {\n\
    gl_Position = u_proj * u_trans * vec4(a_position.x, a_position.y, a_position.z, 1.0);\n\
}\n\
";

constexpr const char *DEFAULT_FRAGMENT_SHADER = "\
#version 150 core\n\
out vec4 fragColor;\n\
\n\
void main() {\n\
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);\n\
}\n\
";

using namespace glm;

namespace Fantasy {
    class SpriteBatch {
        bool batching;
        size_t index;

        Mesh *mesh;
        Shader *shader;
        mat4 projection;
        mat4 transform;
        GLfloat *vertices;

        public:
        SpriteBatch();
        SpriteBatch(size_t, Shader *);
        ~SpriteBatch();

        void begin();
        void end();
    };
}

#endif
