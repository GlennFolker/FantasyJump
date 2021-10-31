#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <entt/entity/registry.hpp>

#include "../app_listener.h"

namespace Fantasy {
    class GameController: public AppListener {
        public:
        entt::registry *regist;

        public:
        GameController();
        ~GameController() override;
        void update() override;
    };
}

#endif
