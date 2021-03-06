#ifndef ENTITY_H
#define ENTITY_H

#include <SDL_mixer.h>
#include <box2d/box2d.h>
#include <entt/entity/registry.hpp>
#include <string>
#include <optional>
#include <functional>

#include "team.h"
#include "../graphics/tex_atlas.h"

namespace Fantasy {
    class Component {
        protected:
        entt::entity ref;

        public:
        Component(entt::entity);

        virtual void update();
        void remove();
        entt::entity createFx(const std::string &, bool follow = false);
        int createSfx(Mix_Chunk *);
        entt::entity getRef();
    };

    class RigidComp: public Component {
        public:
        b2Body *body;
        float rotateSpeed;

        std::string spawnFx, deathFx;
        Mix_Chunk *spawnSfx, *deathSfx;

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

    class DrawComp: public Component {
        public:
        std::string drawer;

        std::optional<TexRegion> region;
        float width, height, z;

        public:
        DrawComp(entt::entity, const std::string &);
        DrawComp(entt::entity, const std::string &, float);
        DrawComp(entt::entity, const std::string &, float, float);
        DrawComp(entt::entity, const std::string &, float, float, float);

        void update() override;
    };

    class JumpComp: public Component {
        public:
        float force, timeout;
        std::string effect;
        Mix_Chunk *sound;

        private:
        bool holding, jumping;
        float time;
        b2Vec2 target;

        public:
        JumpComp(entt::entity, float, float);

        void update() override;
        void hold();
        void release(float, float);
        bool isHolding();
        float getTime();
    };

    class HealthComp: public Component {
        public:
        float health, maxHealth, damage, regeneration, hitTime;
        bool selfDamage, showBar;

        private:
        bool dead;

        public:
        HealthComp(entt::entity, float);
        HealthComp(entt::entity, float, float);
        HealthComp(entt::entity, float, float, float);

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
        float priority;
        Team::TeamType team;

        public:
        TeamComp(entt::entity);
        TeamComp(entt::entity, float);
        TeamComp(entt::entity, Team::TeamType);
        TeamComp(entt::entity, Team::TeamType, float);
    };

    class ShooterComp: public Component {
        public:
        std::string bullet, shootFx;
        Mix_Chunk *shootSfx;
        float rate, impulse, range, inaccuracy;

        private:
        float lastShoot, timer;

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

    class IdentifierComp: public Component {
        public:
        std::string id;

        public:
        IdentifierComp(entt::entity, const std::string &);
    };
}

#endif
