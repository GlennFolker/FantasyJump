#ifndef ENTITY_H
#define ENTITY_H

#include "team.h"
#include "../graphics/tex.h"

#include <box2d/box2d.h>
#include <entt/entity/registry.hpp>
#include <functional>

namespace Fantasy {
    class Component {
        protected:
        entt::entity ref;

        public:
        Component(entt::entity);

        virtual void update();
        void remove();

        entt::entity getRef();
        static entt::registry &getRegistry();
        static b2World &getWorld();
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
        bool shouldCollide(RigidComp &);

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

    class TeamComp: public Component {
        public:
        Team::TeamType team;

        public:
        TeamComp(entt::entity);
        TeamComp(entt::entity, Team::TeamType);
    };

    class ShooterComp: public Component {
        public:
        enum ShootType {
            SMALL,
            MEDIUM
        };

        private:
        float time;

        public:
        ShootType type;
        float rate, impulse, range;

        public:
        ShooterComp(entt::entity, ShootType, float);
        ShooterComp(entt::entity, ShootType, float, float);
        ShooterComp(entt::entity, ShootType, float, float, float);

        void update() override;
    };

    class TemporalComp: public Component {
        public:
        enum TemporalFlag {
            RANGE = 1,
            TIME = 2
        };

        public:
        TemporalFlag flags;
        float range, time;

        private:
        bool init;
        float initTime;
        b2Vec2 initPos;

        public:
        TemporalComp(entt::entity, TemporalFlag);

        void update() override;
    };
}

#endif
