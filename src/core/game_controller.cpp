#include "game_controller.h"

#include "../app.h"

namespace Fantasy {
    Tex2D *texture;

    GameController::GameController() {
        regist = new entt::registry();
    }

    GameController::~GameController() {
        regist->~basic_registry();
    }

    void GameController::update() {
        
    }
}
