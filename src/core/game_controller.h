#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <entt/entity/registry.hpp>
#include <box2d/box2d.h>
#include <unordered_set>

#include "../app_listener.h"
#include "content.h"

namespace Fantasy {
    class GameController: public AppListener, public b2ContactListener, public b2ContactFilter {
        private:
        std::unordered_set<entt::entity> *removal;
        float restartTime;
        float winTime;
        float resetTime;
        float exitTime;
        bool resetting;
        int leakKilled;
        
        public:
        static const float worldWidth;
        static const float worldHeight;
        static const float borderThickness;

        Contents *content;
        b2World *world;
        entt::registry *regist;
        entt::entity player;

        public:
        GameController();
        ~GameController() override;
        void update() override;
        void scheduleRemoval(entt::entity);
        void resetGame();
        bool isResetting();
        float getRestartTime();
        float getWinTime();
        float getResetTime();
        float getExitTime();

        void BeginContact(b2Contact *) override;
        void EndContact(b2Contact *) override;
        bool ShouldCollide(b2Fixture *, b2Fixture *) override;

        private:
        void removeEntities();
    };
}

#endif
