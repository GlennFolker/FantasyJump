#ifndef ENTITY_H
#define ENTITY_H

#include <box2d/box2d.h>
#include <entt/entity/registry.hpp>
#include <functional>

#include "../graphics/tex.h"

namespace Fantasy {
    class Component {
        protected:
        entt::entity ref;

        public:
        Component(entt::entity);

        virtual void update();

        entt::entity getRef();
        entt::registry &getRegistry();
        b2World &getWorld();
    };

    class RigidComp: public Component {
        public:
        b2Body *body;
        float rotateSpeed;

        public:
        RigidComp(entt::entity, b2Body *);
        void update() override;

        void beginCollide(RigidComp &);
        void endCollide(RigidComp &);

        static void onDestroy(entt::registry &, entt::entity);
    };

    class SpriteComp: public Component {
        public:
        Tex2D *texture;
        float width, height, z;

        public:
        SpriteComp(entt::entity, Tex2D *);
        SpriteComp(entt::entity, Tex2D *, float);
        SpriteComp(entt::entity, Tex2D *, float, float);
        SpriteComp(entt::entity, Tex2D *, float, float, float);

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

    class HealthComp: public Component {
        public:
        float health, maxHealth, damage, hitTime;
        bool selfDamage;

        public:
        HealthComp(entt::entity, float);
        HealthComp(entt::entity, float, float);

        public:
        void update() override;
        void kill();
        void killed();
        void heal(float);
        void hurt(float);
        bool canHurt();
    };
}

#endif
