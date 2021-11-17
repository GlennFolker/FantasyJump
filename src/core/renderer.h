#ifndef RENDERER_H
#define RENDERER_H

#include <box2d/box2d.h>

#include "../app_listener.h"
#include "../graphics/sprite_batch.h"
#include "../graphics/tex_atlas.h"
#include "../graphics/frame_buffer.h"
#include "../graphics/shader.h"

namespace Fantasy {
    class Renderer: public AppListener {
        public:
        TexAtlas *atlas;
        SpriteBatch *batch;

        private:
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
