#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H

#include <glm/mat4x4.hpp>

#include "mesh.h"
#include "tex.h"
#include "tex_atlas.h"
#include "color.h"

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

        private:
        size_t vertLength;
        float *vertices;
        float *tmp;
        glm::mat4 projection;

        public:
        SpriteBatch();
        SpriteBatch(size_t, Shader *);
        ~SpriteBatch();
        
        void draw(Tex2D *, float *, size_t, size_t);
        void draw(const TexRegion &, float, float);
        void draw(const TexRegion &, float, float, float rotation = 0.0f);
        void draw(const TexRegion &, float, float, float, float, float rotation = 0.0f);
        void draw(const TexRegion &, float, float, float, float, float, float, float rotation = 0.0f);
        void col(Color);
        void col(float);
        void tint(Color);
        void tint(float);

        void proj(const glm::mat4 &projection);
        void flush();
    };
}

#endif
