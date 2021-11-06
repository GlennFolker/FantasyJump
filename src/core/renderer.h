#ifndef RENDERER_H
#define RENDERER_H

#include "../app_listener.h"
#include "../graphics/sprite_batch.h"
#include "../graphics/frame_buffer.h"

namespace Fantasy {
    class Renderer: public AppListener {
        public:
        SpriteBatch *batch;
        FrameBuffer *buffer;
        Mesh *quad;
        Shader *bloom;

        public:
        Renderer();
        ~Renderer() override;
        void update() override;
    };
}

#endif
