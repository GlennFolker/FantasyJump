#ifndef RENDERER_H
#define RENDERER_H

#include <glm/mat4x4.hpp>
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
        glm::dmat4 proj;
        glm::dmat4 flipProj;
        glm::dvec2 pos;
        glm::dvec2 scl;

        private:
        FrameBuffer *buffer;
        Mesh *quad;
        Shader *bloom;
        std::vector<entt::entity> *toRender;

        public:
        Renderer();
        ~Renderer() override;

        void update() override;
        void unproject(double, double, double *, double *);
        bool ReportFixture(b2Fixture *fixture) override;
    };
}

#endif
