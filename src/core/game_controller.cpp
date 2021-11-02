#include "game_controller.h"

namespace Fantasy {
    GameController::GameController() {
        regist = new entt::registry();
    }

    GameController::~GameController() {
        delete regist;
    }

    void GameController::update() {

    }
}
