#ifndef RENDERER_H
#define RENDERER_H

#include <box2d/box2d.h>

#include "../app_listener.h"
#include "../graphics/sprite_batch.h"
#include "../graphics/frame_buffer.h"

namespace Fantasy {
    class Renderer: public AppListener {
        public:
        SpriteBatch *batch;

        private:
        FrameBuffer *buffer;
        Mesh *quad;
        Shader *bloom;

        Tex2D *bg1;
        Shader *bgShader;

        public:
        Renderer();
        ~Renderer() override;
        void update() override;

        private:
        Tex2D *loadTex(const char *);
    };
}

#endif
