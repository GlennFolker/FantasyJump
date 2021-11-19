#ifndef RENDERER_H
#define RENDERER_H

#include <box2d/box2d.h>
#include <entt/entity/registry.hpp>

#include "../app_listener.h"
#include "../graphics/sprite_batch.h"
#include "../graphics/tex_atlas.h"
#include "../graphics/frame_buffer.h"
#include "../graphics/shader.h"

namespace Fantasy {
    class Renderer: public AppListener, public b2QueryCallback {
        public:
        TexAtlas *atlas;
        SpriteBatch *batch;

        private:
        FrameBuffer *buffer;
        Mesh *quad;
        Shader *bloom;
        std::vector<entt::entity> *toRender;

        public:
        Renderer();
        ~Renderer() override;

        void update() override;
        bool ReportFixture(b2Fixture *fixture) override;
    };
}

#endif
