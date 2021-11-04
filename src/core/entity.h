#ifndef ENTITY_H
#define ENTITY_H
#include <box2d/box2d.h>
#include <entt/entity/registry.hpp>

#include "../graphics/tex.h"

namespace Fantasy {
    class Component {
        protected:
        entt::entity ref;

        public:
        Component(entt::entity);
        virtual void update() = 0;
    };

    class RigidComp: public Component {
        public:
        b2Body *body;

        public:
        RigidComp(entt::entity, b2Body *);
        void update() override;

        static void onDestroy(entt::registry &, entt::entity);
    };

    class SpriteComp: public Component {
        public:
        Tex2D *texture;

        public:
        SpriteComp(entt::entity, Tex2D *);

        void update() override;
    };

    class JumpComp: public Component {
        public:
        float force, timeout;

        private:
        bool holding, jumping;
        float time;
        b2Vec2 target;

        public:
        JumpComp(entt::entity, float, float);

        void hold();
        void release(float, float);
        void update() override;
    };
}

#endif
