#include <SDL.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <entt/entity/observer.hpp>

#include "game_controller.h"
#include "entity.h"
#include "events.h"
#include "time.h"
#include "../app.h"
#include "../util/mathf.h"

namespace Fantasy {
    const float GameController::worldWidth = 400.0f;
    const float GameController::worldHeight = 300.0f;
    const float GameController::borderThickness = 2.0f;

    GameController::GameController() {
        regist = new entt::registry();
        regist->on_destroy<RigidComp>().connect<&RigidComp::onDestroy>();
        removal = new std::unordered_set<entt::entity>();
        resetting = false;

        world = new b2World(b2Vec2(0.0f, -9.81f));
        world->SetContactListener(this);
        world->SetContactFilter(this);

        content = new Contents();

        leakKilled = 0;
        restartTime = winTime = resetTime = exitTime = -1.0f;
        player = entt::entity();
        App::instance->input->attach(Input::MOUSE, [&](InputContext &ctx) {
            if(ctx.read<SDL_MouseButtonEvent>().button != SDL_BUTTON_LEFT || !regist->valid(player)) return;
            
            JumpComp &comp = regist->get<JumpComp>(player);
            if(!ctx.performed) {
                double x, y;
                App::irenderer().unproject(App::instance->getMouseX(), App::instance->getMouseY(), &x, &y);

                comp.release(x, y);
            } else {
                comp.hold();
            }
        });

        App::instance->input->attach(Input::KEYBOARD, [&](InputContext &ctx) {
            switch(ctx.read<SDL_KeyboardEvent>().keysym.scancode) {
                case SDL_SCANCODE_F4: if(ctx.performed) { App::instance->setFullscreen(!App::instance->isFullscreen()); } break;
                case SDL_SCANCODE_ESCAPE: if(ctx.performed) {
                    if(exitTime == -1.0f) exitTime = Time::time();
                } else {
                    exitTime = -1.0f;
                } break;
            }
        });

        Events::on<EntDeathEvent>([this](Event &e) {
            EntDeathEvent &ent = (EntDeathEvent &)e;
            if(winTime == -1.0f && ent.entity == player) {
                restartTime = Time::time();
            } else if(restartTime == -1.0f && regist->any_of<IdentifierComp>(ent.entity) && regist->get<IdentifierComp>(ent.entity).id == "leak" && ++leakKilled >= 3) {
                winTime = Time::time();
            }
        });
    }

    GameController::~GameController() {
        removeEntities();
        delete removal;
        delete regist;
        delete world;
        delete content;
    }

    void GameController::resetGame() {
        resetting = true;
        regist->each([this](const entt::entity e) { regist->destroy(e); });
        regist->clear();
        removal->clear();

        for(int i = -1; i <= 1; i += 2) {
            b2BodyDef bodyDef;
            bodyDef.type = b2_staticBody;
            bodyDef.position.Set(0.0f, i * worldHeight / 2.0f - borderThickness / 2.0f * i);

            b2PolygonShape shape;
            shape.SetAsBox(worldWidth / 2.0f, borderThickness / 2.0f);

            b2FixtureDef fixt;
            fixt.shape = &shape;
            fixt.restitution = 0.93f;
            fixt.restitutionThreshold = 0.0f;

            b2Body *bodyA = world->CreateBody(&bodyDef);
            bodyA->CreateFixture(&fixt);

            entt::entity borderA = regist->create();
            regist->emplace<RigidComp>(borderA, borderA, bodyA);
            regist->emplace<DrawComp>(borderA, borderA, content->genericRegion->name, borderThickness, worldWidth).region = App::iatlas().get("red-box");
            regist->emplace<HealthComp>(borderA, borderA, -1.0f, 10.0f);

            bodyDef.position.Set(i * worldWidth / 2.0f - borderThickness / 2.0f * i, 0.0f);
            shape.SetAsBox(borderThickness / 2.0f, worldHeight / 2.0f);

            b2Body *bodyB = world->CreateBody(&bodyDef);
            bodyB->CreateFixture(&fixt);

            entt::entity borderB = regist->create();
            regist->emplace<RigidComp>(borderB, borderB, bodyB);
            regist->emplace<DrawComp>(borderB, borderB, content->genericRegion->name, worldHeight, borderThickness).region = App::iatlas().get("red-box");
            regist->emplace<HealthComp>(borderB, borderB, -1.0f, 10.0f);
        }

        leakKilled = 0;
        restartTime = winTime = -1.0f;
        resetTime = Time::time();
        player = content->jumper->create();

        std::function<void(entt::entity, float)> adjust = [this](entt::entity e, float range) {
            b2Body *body = regist->get<RigidComp>(e).body;
            do {
                body->SetTransform(b2Vec2(
                    Mathf::random(-worldWidth + borderThickness, worldWidth - borderThickness) / 2.0f,
                    Mathf::random(-worldHeight + borderThickness, worldHeight - borderThickness) / 2.0f
                ), 0.0f);
            } while(body->GetTransform().p.Length() <= range);
        };

        for(int i = 0; i < 3; i++) adjust(content->leak->create(), 56.0f);
        for(int i = 0; i < 500; i++) adjust(content->spike->create(), 16.0f);

        resetting = false;
    }

    void GameController::update() {
        if(exitTime != -1.0f && Time::time() - exitTime >= 1.0f) App::instance->exit();
        if((restartTime != -1.0f && Time::time() - restartTime >= 3.0f) || (winTime != -1.0f && Time::time() - winTime >= 5.0f)) resetGame();
        removeEntities();

        world->Step(1.0f / 60.0f, 1, 1);
        regist->view<RigidComp>().each([](const entt::entity &e, RigidComp &comp) { comp.update(); });
        regist->view<JumpComp>().each([](const entt::entity &e, JumpComp &comp) { comp.update(); });
        regist->view<HealthComp>().each([](const entt::entity &e, HealthComp &comp) { comp.update(); });
        regist->view<ShooterComp>().each([](const entt::entity &e, ShooterComp &comp) { comp.update(); });
        regist->view<TemporalComp>().each([](const entt::entity &e, TemporalComp &comp) { comp.update(); });
    }

    void GameController::BeginContact(b2Contact *contact) {
        b2Fixture *fa = contact->GetFixtureA(), *fb = contact->GetFixtureB();
        if(fa->IsSensor() || fb->IsSensor()) return;

        entt::entity a = (entt::entity)fa->GetBody()->GetUserData().pointer;
        entt::entity b = (entt::entity)fb->GetBody()->GetUserData().pointer;
        if(regist->valid(a) && regist->any_of<RigidComp>(a) && regist->valid(b) && regist->any_of<RigidComp>(b)) {
            RigidComp &first = regist->get<RigidComp>(a), &second = regist->get<RigidComp>(b);
            first.beginCollide(second);
            second.beginCollide(first);
        }
    }

    void GameController::EndContact(b2Contact *contact) {
        b2Fixture *fa = contact->GetFixtureA(), *fb = contact->GetFixtureB();
        if(fa->IsSensor() || fb->IsSensor()) return;

        entt::entity a = (entt::entity)fa->GetBody()->GetUserData().pointer;
        entt::entity b = (entt::entity)fb->GetBody()->GetUserData().pointer;
        if(regist->valid(a) && regist->any_of<RigidComp>(a) && regist->valid(b) && regist->any_of<RigidComp>(b)) {
            RigidComp &first = regist->get<RigidComp>(a), &second = regist->get<RigidComp>(b);
            first.endCollide(second);
            second.endCollide(first);
        }
    }

    bool GameController::ShouldCollide(b2Fixture *fixtA, b2Fixture *fixtB) {
        entt::entity a = (entt::entity)fixtA->GetBody()->GetUserData().pointer;
        entt::entity b = (entt::entity)fixtB->GetBody()->GetUserData().pointer;
        if(regist->valid(a) && regist->any_of<RigidComp>(a) && regist->valid(b) && regist->any_of<RigidComp>(b)) {
            RigidComp &first = regist->get<RigidComp>(a), &second = regist->get<RigidComp>(b);
            return first.shouldCollide(second) || second.shouldCollide(first);
        } else {
            return false;
        }
    }

    void GameController::scheduleRemoval(entt::entity e) { removal->insert(e); }
    void GameController::removeEntities() {
        for(entt::entity e : *removal) regist->destroy(e);
        removal->clear();
    }

    bool GameController::isResetting() { return resetting; }
    float GameController::getWinTime() { return winTime; }
    float GameController::getRestartTime() { return restartTime; }
    float GameController::getResetTime() { return resetTime; }
    float GameController::getExitTime() { return exitTime; }
}
