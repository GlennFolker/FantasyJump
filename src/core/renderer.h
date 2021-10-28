#ifndef RENDERER_H
#define RENDERER_H

#include "../app_listener.h"
#include "../graphics/sprite_batch.h"
#include "../graphics/camera.h"

namespace Fantasy {
    class Renderer: public AppListener {
        public:
        SpriteBatch *batch;
        Camera camera;

        public:
        Renderer();
        ~Renderer();
        void update() override;
    };
}

#endif
