#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <entt/entity/registry.hpp>
#include <box2d/box2d.h>

#include "../app_listener.h"
#include "content.h"

namespace Fantasy {
    class GameController: public AppListener {
        public:
        Contents *content;
        entt::registry *regist;
        b2World *world;

        public:
        GameController();
        ~GameController() override;
        void update() override;
    };
}

#endif
