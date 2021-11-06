#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "game_controller.h"
#include "entity.h"
#include "../app.h"
#include "../util/mathf.h"

namespace Fantasy {
    const float GameController::worldWidth = 800.0f;
    const float GameController::worldHeight = 600.0f;
    const float GameController::borderThickness = 2.0f;

    GameController::GameController() {
        regist = new entt::registry();
        regist->on_destroy<RigidComp>().connect<&RigidComp::onDestroy>();
        removal = new std::unordered_set<entt::entity>();

        world = new b2World(b2Vec2(0.0f, -9.81f));
        world->SetContactListener(this);

        content = new Contents();

        borderTex = new Tex2D("assets/red-box.png");
        borderTex->load();
        borderTex->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);

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
            regist->emplace<SpriteComp>(borderA, borderA, borderTex, worldWidth, borderThickness);
            regist->emplace<HealthComp>(borderA, borderA, -1.0f, 10.0f);

            bodyDef.position.Set(i * worldWidth / 2.0f - borderThickness / 2.0f * i, 0.0f);
            shape.SetAsBox(borderThickness / 2.0f, worldHeight / 2.0f);

            b2Body *bodyB = world->CreateBody(&bodyDef);
            bodyB->CreateFixture(&fixt);

            entt::entity borderB = regist->create();
            regist->emplace<RigidComp>(borderB, borderB, bodyB);
            regist->emplace<SpriteComp>(borderB, borderB, borderTex, borderThickness, worldHeight);
            regist->emplace<HealthComp>(borderB, borderB, -1.0f, 10.0f);
        }

        player = content->jumper->create(*regist, *world);

        JumpComp &comp = regist->get<JumpComp>(player);
        App::instance->input->attach(SDL_MOUSEBUTTONDOWN, [&](InputContext &ctx) {
            if(ctx.read<char>() != SDL_BUTTON_LEFT || !regist->valid(comp.getRef())) return;
            if(!ctx.performed) {
                double x, y;
                App::instance->unproject(App::instance->getMouseX(), App::instance->getMouseY(), &x, &y);

                comp.release(x, y);
            } else {
                comp.hold();
            }
        });

        for(int c = 0; c < 1000; c++) {
            b2Body *body = regist->get<RigidComp>(content->spike->create(*regist, *world)).body;
            do {
                body->SetTransform(b2Vec2(
                    Mathf::random(-worldWidth, worldWidth) / 2.0f,
                    Mathf::random(-worldHeight, worldHeight) / 2.0f
                ), 0.0f);
            } while([&]() {
                const b2Transform &trns = body->GetTransform();
                if(trns.p.LengthSquared() < 25.0f) return true;

                b2Fixture *fixtures = body->GetFixtureList();
                int count = sizeof(fixtures) / sizeof(b2Fixture);

                class Report: public b2QueryCallback {
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
                };

                Report report;
                for(int i = 0; i < count; i++) {
                    b2AABB bound;

                    b2Shape *shape = fixtures[i].GetShape();
                    int count = shape->GetChildCount();
                    for(int j = 0; j < count; j++) {
                        shape->ComputeAABB(&bound, trns, j);

                        world->QueryAABB(&report, bound);
                        if(report.isFound()) return true;
                    }
                }

                return false;
            }());
        }
    }

    GameController::~GameController() {
        delete regist;
        delete world;
        delete content;

        removeEntities();
        delete removal;
        delete borderTex;
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
