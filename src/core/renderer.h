#ifndef RENDERER_H
#define RENDERER_H

#include "../app_listener.h"
#include "../graphics/sprite_batch.h"

namespace Fantasy {
    class Renderer: public AppListener {
        public:
        SpriteBatch *batch;
        vec2 position;

        public:
        Renderer();
        ~Renderer() override;
        void update() override;
    };
}

#endif
