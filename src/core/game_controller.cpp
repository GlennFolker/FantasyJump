#include "game_controller.h"

namespace Fantasy {
    GameController::GameController() {
        regist = new entt::registry();
    }

    GameController::~GameController() {
        regist->~basic_registry();
    }

    void GameController::update() {

    }
}
