#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H

#include <glm/mat4x4.hpp>

#include "mesh.h"
#include "shader.h"

constexpr const char *DEFAULT_VERTEX_SHADER = "\
#version 150 core\n\
in vec3 a_position;\n\
\n\
uniform mat4 u_proj;\n\
\n\
void main() {\n\
    mat4 t = u_proj;\n\
    gl_Position = u_proj * vec4(a_position.x, a_position.y, a_position.z, 1.0);\n\
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
        private:
        size_t index;
        size_t spriteSize;

        Mesh *mesh;
        Shader *shader;
        float *vertices;

        public:
        SpriteBatch();
        SpriteBatch(size_t, Shader *);
        ~SpriteBatch();

        void flush(mat4 projection);
    };
}

#endif
