#ifndef ENTITY_H
#define ENTITY_H

#include "team.h"
#include "../graphics/tex_atlas.h"

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
        void applyFx(const std::string &);
        static entt::entity createFx(const std::string &);

        entt::entity getRef();
        static entt::registry &getRegistry();
        static b2World &getWorld();
    };

    class RigidComp: public Component {
        public:
        b2Body *body;
        float rotateSpeed;

        std::string spawnFx, deathFx;

        private:
        bool spawned;

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
        TexRegion region;
        float width, height, z;

        public:
        SpriteComp(entt::entity, const TexRegion &);
        SpriteComp(entt::entity, const TexRegion &, float);
        SpriteComp(entt::entity, const TexRegion &, float, float);
        SpriteComp(entt::entity, const TexRegion &, float, float, float);

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
        std::string bullet;
        float rate, impulse, range;

        private:
        float time;

        public:
        ShooterComp(entt::entity, const std::string &, float);
        ShooterComp(entt::entity, const std::string &, float, float);
        ShooterComp(entt::entity, const std::string &, float, float, float);

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
        float initTime;
        float travelled;

        public:
        TemporalComp(entt::entity, TemporalFlag);

        void update() override;
        float rangef();
        float timef();
    };

    class EffectComp: public Component {
        public:
        float z;
        std::string effect;

        public:
        EffectComp(entt::entity);
        EffectComp(entt::entity, const std::string &);

        void update() override;
    };
}

#endif
