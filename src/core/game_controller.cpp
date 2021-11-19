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

        restartTime = -1.0f;
        player = entt::entity();
        App::instance->input->attach(Input::MOUSE, [&](InputContext &ctx) {
            if(ctx.read<SDL_MouseButtonEvent>().button != SDL_BUTTON_LEFT || !regist->valid(player)) return;
            
            JumpComp &comp = regist->get<JumpComp>(player);
            if(!ctx.performed) {
                double x, y;
                App::instance->unproject(App::instance->getMouseX(), App::instance->getMouseY(), &x, &y);

                comp.release(x, y);
            } else {
                comp.hold();
            }
        });

        App::instance->input->attach(Input::KEYBOARD, [](InputContext &ctx) {
            if(ctx.read<SDL_KeyboardEvent>().keysym.scancode == SDL_SCANCODE_ESCAPE) App::instance->exit();
        });

        Events::on<EntDeathEvent>([this](Event &e) {
            EntDeathEvent &ent = (EntDeathEvent &)e;
            if(ent.entity == player) restartTime = Time::time();
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
            regist->emplace<DrawComp>(borderA, borderA, content->genericRegion->name, worldWidth, borderThickness).region = App::iatlas().get("red-box");
            regist->emplace<HealthComp>(borderA, borderA, -1.0f, 10.0f);

            bodyDef.position.Set(i * worldWidth / 2.0f - borderThickness / 2.0f * i, 0.0f);
            shape.SetAsBox(borderThickness / 2.0f, worldHeight / 2.0f);

            b2Body *bodyB = world->CreateBody(&bodyDef);
            bodyB->CreateFixture(&fixt);

            entt::entity borderB = regist->create();
            regist->emplace<RigidComp>(borderB, borderB, bodyB);
            regist->emplace<DrawComp>(borderB, borderB, content->genericRegion->name, borderThickness, worldHeight).region = App::iatlas().get("red-box");
            regist->emplace<HealthComp>(borderB, borderB, -1.0f, 10.0f);
        }

        restartTime = -1.0f;
        player = content->jumper->create();

        for(int c = 0; c < 500; c++) {
            b2Body *body = regist->get<RigidComp>(content->spike->create()).body;
            do {
                body->SetTransform(b2Vec2(
                    Mathf::random(-worldWidth + borderThickness, worldWidth - borderThickness) / 2.0f,
                    Mathf::random(-worldHeight + borderThickness, worldHeight - borderThickness) / 2.0f
                ), 0.0f);
            } while([&]() {
                const b2Transform &trns = body->GetTransform();
                if(trns.p.LengthSquared() < 25.0f) return true;

                class: public b2QueryCallback {
                    private:
                    bool found = false;

                    public:
                    bool ReportFixture(b2Fixture *fixture) override {
                        found = true;
                        return false;
                    }

                    bool isFound() {
                        return found;
                    }
                } report;

                for(b2Fixture *fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
                    b2AABB bound;

                    b2Shape *shape = fixture->GetShape();
                    int scount = shape->GetChildCount();
                    for(int j = 0; j < scount; j++) {
                        shape->ComputeAABB(&bound, trns, j);
                        bound.lowerBound *= 2.0f;
                        bound.upperBound *= 2.0f;

                        world->QueryAABB(&report, bound);
                        if(report.isFound()) return true;
                    }
                }

                return false;
            }());
        }

        resetting = false;
    }

    void GameController::update() {
        if(restartTime != -1.0f && Time::time() - restartTime > 3.0f) resetGame();
        removeEntities();

        world->Step(1.0f / 60.0f, 1, 1);
        regist->each([this](const entt::entity e) {
            if(!regist->valid(e)) return;
            if(regist->any_of<RigidComp>(e)) regist->get<RigidComp>(e).update();
            if(regist->any_of<JumpComp>(e)) regist->get<JumpComp>(e).update();
            if(regist->any_of<HealthComp>(e)) regist->get<HealthComp>(e).update();
            if(regist->any_of<ShooterComp>(e)) regist->get<ShooterComp>(e).update();
            if(regist->any_of<TemporalComp>(e)) regist->get<TemporalComp>(e).update();
        });
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

    void GameController::scheduleRemoval(entt::entity e) {
        removal->insert(e);
    }

    void GameController::removeEntities() {
        for(entt::entity e : *removal) regist->destroy(e);
        removal->clear();
    }

    bool GameController::isResetting() {
        return resetting;
    }
}
