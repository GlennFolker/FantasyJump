#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H

#include <glm/mat4x4.hpp>

#include "mesh.h"
#include "tex.h"
#include "color.h"

using namespace glm;

namespace Fantasy {
    class SpriteBatch {
        public:
        float z;

        protected:
        Tex2D *texture;
        Color color;
        float colorBits;
        Color tinted;
        float tintBits;

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
        void col(Color);
        void col(float);
        void tint(Color);
        void tint(float);

        void proj(mat4 projection);
        void flush();
    };
}

#endif
