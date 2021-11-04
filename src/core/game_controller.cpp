#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "game_controller.h"
#include "entity.h"
#include "../app.h"

namespace Fantasy {
    entt::entity player;

    GameController::GameController() {
        regist = new entt::registry();
        regist->on_destroy<RigidComp>().connect<RigidComp::onDestroy>();

        world = new b2World(b2Vec2(0.0f, -9.81f));
        content = new Contents();

        player = content->jumper->create(*regist, *world);

        JumpComp &comp = regist->get<JumpComp>(player);
        App::instance->input->attach(SDL_MOUSEBUTTONDOWN, [&comp](InputContext &ctx) {
            if(!ctx.performed) {
                double x, y;
                App::instance->unproject(App::instance->getMouseX(), App::instance->getMouseY(), &x, &y);

                comp.release(x, y);
            } else {
                comp.hold();
            }
        });

        regist->get<RigidComp>(content->spike->create(*regist, *world)).body->SetTransform(b2Vec2(-6.0f, -6.0f), 0.0f);
        regist->get<RigidComp>(content->spike->create(*regist, *world)).body->SetTransform(b2Vec2(6.0f, -6.0f), 0.0f);
        regist->get<RigidComp>(content->spike->create(*regist, *world)).body->SetTransform(b2Vec2(6.0f, 6.0f), 0.0f);
        regist->get<RigidComp>(content->spike->create(*regist, *world)).body->SetTransform(b2Vec2(-6.0f, 6.0f), 0.0f);
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
            if(regist->any_of<JumpComp>(e)) regist->get<JumpComp>(e).update();
        });

        b2Vec2 pos = regist->get<RigidComp>(player).body->GetPosition();
        App::instance->pos = vec2(pos.x, pos.y);
    }
}
