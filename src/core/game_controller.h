#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <entt/entity/registry.hpp>
#include <box2d/box2d.h>
#include <unordered_set>

#include "../app_listener.h"
#include "content.h"

namespace Fantasy {
    class GameController: public AppListener, public b2ContactListener {
        private:
        std::unordered_set<entt::entity> *removal;
        float restartTime;

        public:
        static const float worldWidth;
        static const float worldHeight;
        static const float borderThickness;

        Tex2D *borderTex;
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

        void BeginContact(b2Contact *contact) override;
        void EndContact(b2Contact *contact) override;

        private:
        void removeEntities();
    };
}

#endif
