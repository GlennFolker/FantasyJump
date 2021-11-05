#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "game_controller.h"
#include "entity.h"
#include "../app.h"

namespace Fantasy {
    entt::entity player;

    GameController::GameController() {
        regist = new entt::registry();
        regist->on_destroy<RigidComp>().connect<&RigidComp::onDestroy>();
        removal = new std::unordered_set<entt::entity>();

        world = new b2World(b2Vec2(0.0f, -9.81f));
        world->SetContactListener(this);

        content = new Contents();

        player = content->jumper->create(*regist, *world);

        JumpComp &comp = regist->get<JumpComp>(player);
        App::instance->input->attach(SDL_MOUSEBUTTONDOWN, [&](InputContext &ctx) {
            if(!regist->valid(comp.getRef())) return;
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

        removeEntities();
        delete removal;
    }

    void GameController::update() {
        removeEntities();

        world->Step(1.0f / 60.0f, 8, 3);
        regist->each([this](const entt::entity e) {
            if(!regist->valid(e)) return;
            if(regist->any_of<RigidComp>(e)) regist->get<RigidComp>(e).update();
            if(regist->any_of<JumpComp>(e)) regist->get<JumpComp>(e).update();
            if(regist->any_of<HealthComp>(e)) regist->get<HealthComp>(e).update();
        });

        if(regist->valid(player)) {
            b2Vec2 pos = regist->get<RigidComp>(player).body->GetPosition();
            App::instance->pos = vec2(pos.x, pos.y);
        }
    }

    void GameController::BeginContact(b2Contact *contact) {
        entt::entity a = (entt::entity)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        entt::entity b = (entt::entity)contact->GetFixtureB()->GetBody()->GetUserData().pointer;
        if(regist->valid(a) && regist->any_of<RigidComp>(a) && regist->valid(b) && regist->any_of<RigidComp>(b)) {
            RigidComp &first = regist->get<RigidComp>(a), &second = regist->get<RigidComp>(b);
            first.beginCollide(second);
            second.beginCollide(first);
        }
    }

    void GameController::EndContact(b2Contact *contact) {
        entt::entity a = (entt::entity)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        entt::entity b = (entt::entity)contact->GetFixtureB()->GetBody()->GetUserData().pointer;
        if(regist->valid(a) && regist->any_of<RigidComp>(a) && regist->valid(b) && regist->any_of<RigidComp>(b)) {
            RigidComp &first = regist->get<RigidComp>(a), &second = regist->get<RigidComp>(b);
            first.endCollide(second);
            second.endCollide(first);
        }
    }

    void GameController::scheduleRemoval(entt::entity e) {
        removal->insert(e);
    }

    void GameController::removeEntities() {
        for(entt::entity e : *removal) regist->destroy(e);
        removal->clear();
    }
}
