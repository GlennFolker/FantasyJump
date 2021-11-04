#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "game_controller.h"
#include "entity.h"
#include "../app.h"

namespace Fantasy {
    GameController::GameController() {
        regist = new entt::registry();
        regist->on_destroy<RigidComp>().connect<RigidComp::onDestroy>();

        world = new b2World(b2Vec2(0.0f, -9.81f));
        content = new Contents();

        JumpComp &comp = regist->get<JumpComp>(content->jumper->create(*regist, *world));
        App::instance->input->attach(SDL_MOUSEBUTTONDOWN, [&comp](InputContext &ctx) {
            if(!ctx.performed) {
                double x, y;
                App::instance->unproject(App::instance->getMouseX(), App::instance->getMouseY(), &x, &y);

                comp.release(x, y);
            } else {
                comp.hold();
            }
        });
    }

    GameController::~GameController() {
        delete regist;
        delete world;
        delete content;
    }

    void GameController::update() {
        world->Step(1.0f / 60.0f, 8, 3);
        regist->each([this](const entt::entity e) {
            if(regist->any_of<RigidComp>(e)) regist->get<RigidComp>(e).update();
            if(regist->any_of<RigidComp>(e)) regist->get<JumpComp>(e).update();
        });
    }
}
