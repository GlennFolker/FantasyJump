#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "content.h"
#include "entity.h"
#include "time.h"
#include "../util/mathf.h"
#include "../app.h"

namespace Fantasy {
    Contents::Contents() {
        contents = new std::vector<std::unordered_map<std::string, Content *> *>((int)CType::ALL);

        sfxShootSmall = loadSound("shoot-small");
        sfxShootMed = loadSound("shoot-medium");
        sfxShootEnergy = loadSound("shoot-energy");
        sfxShootSummon = loadSound("shoot-summon");
        sfxExplodeSmall = loadSound("explode-small");
        sfxExplodeMed = loadSound("explode-medium");
        sfxExplodeBig = loadSound("explode-big");

        genericRegion = create<DrawType>("drawer-generic-region", [](entt::entity e) {
            entt::registry &registry = App::iregistry();

            DrawComp &comp = registry.get<DrawComp>(e);
            if(!comp.region) return;
            
            b2Body *body = registry.get<RigidComp>(e).body;
            b2Vec2 pos = body->GetPosition();
            App::ibatch().draw(*comp.region, pos.x, pos.y, comp.width, comp.height, body->GetAngle() - glm::radians(90.0f));
        });

        drawJumper = create<DrawType>("drawer-jumper", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();
            SpriteBatch &batch = App::ibatch();

            DrawComp &comp = registry.get<DrawComp>(e);
            JumpComp &jump = registry.get<JumpComp>(e);
            b2Body *body = registry.get<RigidComp>(e).body;
            b2Vec2 pos = body->GetPosition();

            if(jump.isHolding()) {
                float frac = fminf((Time::time() - jump.getTime()) / jump.timeout, 1.0f);

                Mathf::randVecs(jump.getTime() * 100.0f, 7, 3.0f, powf(1.0f - frac, 2.0f), [&](float x, float y) {
                    float size = frac * 0.75f;

                    batch.col(Color(Color::blue).lerp(Color::white, frac));
                    batch.draw(atlas.get("white"), pos.x + x, pos.y + y, size, size, glm::radians(45.0f));
                });

                glm::dvec2 mpos;
                App::irenderer().unproject(App::instance->getMouseX(), App::instance->getMouseY(), &mpos.x, &mpos.y);
                mpos -= glm::dvec2(pos.x, pos.y);

                float llen = (1.0f - powf(1.0f - frac, 3.0f)) * 5.0f;
                float langle = glm::orientedAngle(glm::dvec2(1.0f, 0.0f), glm::normalize(mpos));
                batch.col(Color(Color::blue.r, Color::blue.g, Color::blue.b, 0.0f).lerp(Color::lpurple, 1.0f - powf(1.0f - frac, 2.0f)));
                batch.draw(atlas.get("white"), pos.x, pos.y, pos.x - 0.075, pos.y - 0.075, llen, 0.125f, langle);

                float lx = pos.x + cosf(langle) * llen, ly = pos.y + sinf(langle) * llen;
                batch.draw(atlas.get("white"), lx, ly, lx, ly - 0.075f, 1.0f, 0.125f, langle + glm::radians(150.0f));
                batch.draw(atlas.get("white"), lx, ly, lx, ly - 0.075f, 1.0f, 0.125f, langle - glm::radians(150.0f));

                float size = 1.4f + sinf(Time::time() * 20.0f) * 0.2f;
                batch.col(Color(1.0f, 1.0f, 1.0f, powf(frac, 3.0f) * 0.5f));
                batch.draw(atlas.get("jumper"), pos.x, pos.y, size, size, body->GetAngle() - glm::radians(90.0f));
                batch.col(Color::white);
            }

            batch.draw(atlas.get("jumper"), pos.x, pos.y, 1.0f, 1.0f, body->GetAngle() - glm::radians(90.0f));
        });

        jumper = create<EntityType>("ent-jumper", [this](entt::entity e) {
            entt::registry &registry = App::iregistry();

            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();

            b2CircleShape shape;
            shape.m_radius = 0.5f;

            b2FixtureDef fixt;
            fixt.shape = &shape;
            fixt.density = 5.0f;
            fixt.friction = 0.1f;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.deathFx = destructMed->name;
            comp.deathSfx = sfxExplodeMed;

            ShooterComp &shoot = registry.emplace<ShooterComp>(e, e, bulletSmall->name, 0.24f);
            shoot.shootFx = smokeSmall->name;
            shoot.shootSfx = sfxShootSmall;

            registry.emplace<DrawComp>(e, e, drawJumper->name, 1.0f, 1.0f, 2.0f);
            registry.emplace<JumpComp>(e, e, 100.0f, 0.5f).effect = jumped->name;
            registry.emplace<HealthComp>(e, e, 150.0f, 5.0f, 0.04f);
            registry.emplace<TeamComp>(e, e, Team::AZURE, 10.0f);
        });

        spike = create<EntityType>("ent-spike", [this](entt::entity e) {
            entt::registry &registry = App::iregistry();

            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();
            bodyDef.gravityScale = 0.0f;

            b2CircleShape shape;
            shape.m_radius = 0.9f;

            b2FixtureDef fixt;
            fixt.restitution = 1.0f;
            fixt.restitutionThreshold = 0.0f;
            fixt.shape = &shape;
            fixt.density = 10.0f;
            fixt.friction = 0.3f;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);
            
            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.rotateSpeed = glm::radians(Mathf::random(1.0f, 2.5f) * (Mathf::random() >= 0.5f ? 1.0f : -1.0f));
            comp.deathFx = destructBig->name;
            comp.deathSfx = sfxExplodeMed;

            ShooterComp &shoot = registry.emplace<ShooterComp>(e, e, bulletMed->name, 1.2f, 5.0f, 10.0f);
            shoot.shootFx = smokeBig->name;
            shoot.shootSfx = sfxShootMed;

            registry.emplace<DrawComp>(e, e, genericRegion->name, 2.0f, 2.0f, 1.0f).region = App::iatlas().get("spike");
            registry.emplace<HealthComp>(e, e, 100.0f, 10.0f);
            registry.emplace<TeamComp>(e, e, Team::KAYDE, 15.0f);
        });

        drawLeak = create<DrawType>("drawer-ent-leak", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();
            SpriteBatch &batch = App::ibatch();

            b2Vec2 pos = registry.get<RigidComp>(e).body->GetPosition();

            const TexRegion regions[] = {atlas.get("leak-4"), atlas.get("leak-3"), atlas.get("leak-2"), atlas.get("leak-1")};
            float speeds[] = {0.25f, 0.33f, 0.5f, 1.0f};
            float direction = (unsigned int)e % 2 == 0 ? 1.0f : -1.0f;
            for(int i = 0; i < 4; i++) {
                const TexRegion &region = regions[i];

                float rot = fmodf(speeds[i] * direction * Time::time() * glm::pi<float>() + Mathf::srandom((unsigned int)e + i, glm::pi<float>()), glm::two_pi<float>());
                batch.draw(region, pos.x, pos.y, region.width / 8.0f, region.height / 8.0f, rot);
            }

            Mathf::randVecs((unsigned int)e, 64,
                4.0f, 20.0f, 1.0f,
                0.2f * direction * Time::time() * glm::two_pi<float>(),
                [](float len) { return 0.08f + (1.0f - (len - 4.0f) / 16.0f) * 0.92f; },
                [&](float x, float y) {
                    float dst = glm::length(glm::vec2(x, y)) / 20.0f;

                    batch.col(Color(Color::lyellow).lerp(Color(Color::purple.r, Color::purple.g, Color::purple.b, 0.2f), 1.0f - powf(1.0f - dst, 1.5f)));
                    batch.draw(atlas.get("white"), pos.x + x, pos.y + y, 0.25f - dst * 0.125f, 3.5f, glm::orientedAngle(glm::vec2(1.0f, 0.0f), glm::normalize(glm::vec2(x, y))));
                    batch.col(Color::white);
                }
            );
        });

        leak = create<EntityType>("ent-leak", [this](entt::entity e) {
            entt::registry &registry = App::iregistry();

            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();
            bodyDef.gravityScale = 0.0f;

            b2CircleShape shape;
            shape.m_radius = 5.0f;

            b2FixtureDef fixt;
            fixt.shape = &shape;
            fixt.density = 1000.0f;
            fixt.friction = 0.8f;

            b2CircleShape clipSize;
            clipSize.m_radius = 24.0f;

            b2FixtureDef clip;
            clip.shape = &clipSize;
            clip.isSensor = true;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);
            body->CreateFixture(&clip);

            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.deathFx = leaked->name;
            comp.deathSfx = sfxExplodeBig;

            ShooterComp &shoot = registry.emplace<ShooterComp>(e, e, bulletLeak->name, 0.84f, 24.0f, 48.0f);
            shoot.shootFx = laserDefuse->name;
            shoot.shootSfx = sfxShootSummon;

            registry.emplace<HealthComp>(e, e, 480.0f, 150.0f);
            registry.emplace<TeamComp>(e, e, Team::KAYDE, 30.0f);
            registry.emplace<DrawComp>(e, e, drawLeak->name, 1.0f, 1.0f, 2.5f);
            registry.emplace<IdentifierComp>(e, e, "leak");
        });

        bulletSmall = create<EntityType>("ent-bullet-small", [this](entt::entity e) {
            entt::registry &registry = App::iregistry();

            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();
            bodyDef.bullet = true;
            bodyDef.gravityScale = 0.01f;

            b2CircleShape shape;
            shape.m_radius = 0.25f;

            b2FixtureDef fixt;
            fixt.density = 1.0f;
            fixt.shape = &shape;
            fixt.friction = 0.05f;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.deathFx = destructSmall->name;
            comp.deathSfx = sfxExplodeSmall;

            HealthComp &health = registry.emplace<HealthComp>(e, e, 5.0f, 10.0f);
            health.selfDamage = true;
            health.showBar = false;

            registry.emplace<DrawComp>(e, e, genericRegion->name, 0.5f, 0.5f, 3.0f).region = App::iatlas().get("bullet-small");
            registry.emplace<TeamComp>(e, e, 1.0f);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });

        bulletMed = create<EntityType>("ent-bullet-medium", [this](entt::entity e) {
            entt::registry &registry = App::iregistry();

            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();
            bodyDef.bullet = true;
            bodyDef.gravityScale = 0.04f;

            b2CircleShape shape;
            shape.m_radius = 0.5f;

            b2FixtureDef fixt;
            fixt.density = 1.0f;
            fixt.shape = &shape;
            fixt.friction = 0.05f;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.rotateSpeed = glm::radians(Mathf::random() > 0.5f ? 10.0f : -10.0f);
            comp.deathFx = destructSmall->name;
            comp.deathSfx = sfxExplodeSmall;

            HealthComp &health = registry.emplace<HealthComp>(e, e, 10.0f, 20.0f);
            health.selfDamage = true;
            health.showBar = false;

            registry.emplace<DrawComp>(e, e, genericRegion->name, 0.75f, 0.75f, 3.0f).region = App::iatlas().get("bullet-medium");
            registry.emplace<TeamComp>(e, e, 2.0f);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });

        bulletLeak = create<EntityType>("ent-bullet-leak", [this](entt::entity e) {
            entt::registry &registry = App::iregistry();

            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();
            bodyDef.bullet = true;
            bodyDef.gravityScale = 0.0f;

            b2CircleShape shape;
            shape.m_radius = 0.75f;

            b2FixtureDef fixt;
            fixt.density = 1.0f;
            fixt.shape = &shape;
            fixt.friction = 0.05f;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.deathFx = destructBig->name;
            comp.deathSfx = sfxExplodeMed;

            ShooterComp &shoot = registry.emplace<ShooterComp>(e, e, laser->name, 0.5f);
            shoot.shootFx = smokeBig->name;
            shoot.shootSfx = sfxShootEnergy;

            registry.emplace<DrawComp>(e, e, genericRegion->name, 1.25f, 1.25f, 3.5f).region = App::iatlas().get("bullet-leak");
            registry.emplace<HealthComp>(e, e, 100.0f, 100.0f);
            registry.emplace<TeamComp>(e, e, 20.0f);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });

        laser = create<EntityType>("ent-laser", [this](entt::entity e) {
            entt::registry &registry = App::iregistry();

            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();
            bodyDef.bullet = true;
            bodyDef.gravityScale = 0.0f;

            b2PolygonShape shape;
            shape.SetAsBox(0.125f, 1.5f);

            b2FixtureDef fixt;
            fixt.density = 0.2f;
            fixt.shape = &shape;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);
            
            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.deathFx = laserDefuse->name;
            comp.deathSfx = sfxExplodeSmall;

            HealthComp &health = registry.emplace<HealthComp>(e, e, 5.0f, 10.0f);
            health.selfDamage = true;
            health.showBar = false;

            registry.emplace<DrawComp>(e, e, genericRegion->name, 0.25f, 2.0f, 3.0f).region = App::iatlas().get("laser");
            registry.emplace<TeamComp>(e, e, 10.0f);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });

        jumped = create<EffectType>("fx-jumped", create<DrawType>("drawer-fx-jumped", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();
            SpriteBatch &batch = App::ibatch();

            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);
            b2Vec2 pos = comp.body->GetPosition();

            float l = 1.0f - powf(1.0f - life.timef(), 2.5f);

            App::ibatch().col(Color(Color::lpurple).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 12, 2.0f, l, [&](float x, float y) {
                float s = (1.0f - l);
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, s, s);
            });

            float size = (1.0f - l) * 2.5f;
            batch.col(Color(Color::white).lerp(Color(Color::blue.r, Color::blue.g, Color::blue.b, 0.0f), 1.0f - l));
            batch.draw(atlas.get("white"), pos.x, pos.y, size, size, glm::radians(45.0f));

            size = l * 2.5f;
            batch.col(Color(Color::blue).lerp(Color(), l));
            batch.draw(atlas.get("white"), pos.x, pos.y, size, size);
            batch.col(Color::white);
        }));
        jumped->clipSize = 3.0f;
        jumped->lifetime = 0.5f;
        jumped->z = 7.0f;

        smokeSmall = create<EffectType>("fx-smoke-small", create<DrawType>("drawer-fx-smoke-small", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();

            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);
            b2Vec2 pos = comp.body->GetPosition();

            float l = life.timef();
            App::ibatch().col(Color(Color::lyellow).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 3, 1.2f, l, [&](float x, float y) {
                float s = (1.0f - powf(l, 2.0f)) * 0.32f;
                App::ibatch().draw(atlas.get("white"), pos.x + x, pos.y + y, s, s, glm::radians(45.0f));
            });
            App::ibatch().col(Color::white);
        }));
        smokeSmall->clipSize = 2.5f;
        smokeSmall->lifetime = 0.24f;
        smokeSmall->z = 5.0f;

        smokeBig = create<EffectType>("fx-smoke-big", create<DrawType>("drawer-fx-smoke-big", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();

            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);
            b2Vec2 pos = comp.body->GetPosition();

            float l = life.timef();
            App::ibatch().col(Color(Color::lyellow).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 4, 2.0f, l, [&](float x, float y) {
                float s = (1.0f - powf(l, 3.0f)) * 0.5f;
                App::ibatch().draw(atlas.get("white"), pos.x + x, pos.y + y, s, s, glm::radians(45.0f));
            });
            App::ibatch().col(Color::white);
        }));
        smokeBig->clipSize = 5.0f;
        smokeBig->lifetime = 0.4f;
        smokeBig->z = 6.0f;

        destructSmall = create<EffectType>("fx-destruct-small", create<DrawType>("drawer-fx-destruct-small", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();
            SpriteBatch &batch = App::ibatch();

            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);
            b2Vec2 pos = comp.body->GetPosition();

            float l = 1.0f - powf(1.0f - life.timef(), 2.0f);

            batch.col(Color(Color::lyellow).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 6, 1.2f, l, [&](float x, float y) {
                float s = (1.0f - l) * 0.4f;
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, s, s);
            });

            float size = l * 1.2f;
            batch.col(Color(Color::yellow).lerp(Color(), l));
            batch.draw(atlas.get("white"), pos.x, pos.y, size, size);
            batch.col(Color::white);
        }));
        destructSmall->clipSize = 2.5f;
        destructSmall->lifetime = 0.24f;
        destructSmall->z = 8.0f;

        destructMed = create<EffectType>("fx-destruct-medium", create<DrawType>("drawer-fx-destruct-medium", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();
            SpriteBatch &batch = App::ibatch();

            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);
            b2Vec2 pos = comp.body->GetPosition();

            float l = 1.0f - powf(1.0f - life.timef(), 2.5f);

            App::ibatch().col(Color(Color::lorange).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 12, 4.0f, l, [&](float x, float y) {
                float s = (1.0f - l) * 1.3f;
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, s, s);
            });

            float size = l * 5.0f;
            batch.col(Color(Color::orange).lerp(Color(), l));
            batch.draw(atlas.get("white"), pos.x, pos.y, size, size);
            batch.col(Color::white);
        }));
        destructMed->clipSize = 10.0f;
        destructMed->lifetime = 0.8f;
        destructMed->z = 8.0f;

        destructBig = create<EffectType>("fx-destruct-big", create<DrawType>("drawer-fx-destruct-big", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();
            SpriteBatch &batch = App::ibatch();

            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);
            b2Vec2 pos = comp.body->GetPosition();

            float l = 1.0f - powf(1.0f - life.timef(), 3.0f);

            batch.col(Color(Color::lred).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 17, 7.0f, l, [&](float x, float y) {
                float s = (1.0f - l) * 2.0f;
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, s, s);
            });

            float size = l * 7.0f;
            batch.col(Color(Color::red).lerp(Color(), l));
            batch.draw(atlas.get("white"), pos.x, pos.y, size, size);
            batch.col(Color::white);
        }));
        destructBig->clipSize = 15.0f;
        destructBig->lifetime = 1.5f;
        destructBig->z = 9.0f;

        laserDefuse = create<EffectType>("fx-laser-defuse", create<DrawType>("drawer-fx-laser-defuse", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            SpriteBatch &batch = App::ibatch();
            TexAtlas &atlas = App::iatlas();
            b2Vec2 pos = registry.get<RigidComp>(e).body->GetPosition();
            
            float l = powf(1.0f - registry.get<TemporalComp>(e).timef(), 3.0f);

            batch.col(Color(Color::lpurple).lerp(Color::purple, l));
            Mathf::randVecs((unsigned int)e, 5, 2.0f, l, [&](float x, float y) {
                float size = (1.0f - fabs((1.0f - l - 1.0f) * 2.0f + 1.0f)) * 0.5f;
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, size, size, glm::radians(45.0f));
            });

            batch.draw(atlas.get("white"), pos.x, pos.y, l * 1.25f, l * 1.25f);
            batch.col(Color::white);
        }));
        laserDefuse->lifetime = 0.8f;
        laserDefuse->z = 5.5f;

        leaked = create<EffectType>("fx-leaked", create<DrawType>("drawer-fx-leaked", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            SpriteBatch &batch = App::ibatch();
            TexAtlas &atlas = App::iatlas();
            b2Vec2 pos = registry.get<RigidComp>(e).body->GetPosition();

            float time = registry.get<TemporalComp>(e).timef();

            float l = 1.0f - powf(1.0f - Mathf::clamp(time * 6.0f), 5.0f);
            batch.col(Color(Color::lpurple).lerp(Color(Color::purple.r, Color::purple.g, Color::purple.b, 0.0f), l));
            batch.draw(atlas.get("white"), pos.x, pos.y, 8.0f + l * 48.0f, 8.0f + l * 48.0f);

            l = 1.0f - powf(1.0f - Mathf::clamp(time * 3.6f), 5.0f);
            batch.col(Color(Color::lpurple).lerp(Color(Color::purple.r, Color::purple.g, Color::purple.b, 0.0f), l));
            batch.draw(atlas.get("white"), pos.x, pos.y, 8.0f + l * 28.0f, 8.0f + l * 28.0f, glm::radians(45.0f));

            l = 1.0f - powf(1.0f - Mathf::clamp(time * 2.0f), 4.0f);
            Mathf::randVecs((unsigned int)e, 32, 28.0f, l, [&](float x, float y) {
                batch.col(Color(Color::lyellow).lerp(Color::purple, Mathf::clamp(time * 5.0f)));
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, pos.x + x, pos.y + y, 0.5f, 7.5f * (1.0f - l), glm::orientedAngle(glm::vec2(1.0f, 0.0f), glm::normalize(glm::vec2(x, y))));
            });

            l = 1.0f - powf(1.0f - Mathf::clamp(time * 1.5f), 3.0f);
            Mathf::randVecs((unsigned int)e + 1, 24, 17.0f, l, [&](float x, float y) {
                batch.col(Color(Color::lyellow).lerp(Color::purple, Mathf::clamp(time * 5.0f)));
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, pos.x + x, pos.y + y, 0.375f, 5.0f * (1.0f - l), glm::orientedAngle(glm::vec2(1.0f, 0.0f), glm::normalize(glm::vec2(x, y))));
            });

            l = 1.0f - powf(1.0f - Mathf::clamp(time * 1.6f), 3.2f);
            Mathf::randVecs((unsigned int)e + 2, 24, 25.0f, l, [&](float x, float y) {
                float size = (1.0f - l) * 4.8f;

                batch.col(Color(Color::purple).lerp(Color::gray, Mathf::clamp(time * 1.6f)));
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, size, size, glm::radians(45.0f));
            });

            l = 1.0f - powf(1.0f - Mathf::clamp(time * 1.3f), 2.4f);
            Mathf::randVecs((unsigned int)e + 3, 18, 16.0f, l, [&](float x, float y) {
                float size = (1.0f - l) * 6.0f;

                batch.col(Color(Color::lpurple).lerp(Color::gray, Mathf::clamp(time * 1.3f)));
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, size, size, glm::radians(45.0f));
            });

            l = 1.0f - powf(1.0f - time, 2.0f);
            Mathf::randVecs((unsigned int)e + 4, 18, 20.0f, l, [&](float x, float y) {
                float size = (1.0f - l) * 5.6f;

                batch.col(Color(Color::lyellow).lerp(Color::purple, 1.0f - powf(1.0f - time, 3.0f)));
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, size, size, glm::radians(45.0f));
            });

            batch.col(Color::white);
        }));
        leaked->clipSize = 56.0f;
        leaked->lifetime = 5.6f;
        leaked->z = 10.0f;
    }

    Contents::~Contents() {
        delete contents;
        Mix_FreeChunk(sfxShootSmall);
        Mix_FreeChunk(sfxShootMed);
        Mix_FreeChunk(sfxShootEnergy);
        Mix_FreeChunk(sfxShootSummon);
        Mix_FreeChunk(sfxExplodeSmall);
        Mix_FreeChunk(sfxExplodeMed);
        Mix_FreeChunk(sfxExplodeBig);
    }

    std::unordered_map<std::string, Content *> *Contents::getBy(CType type) {
        int ordinal = (int)type;
        if(contents->at(ordinal) == nullptr) contents->at(ordinal) = new std::unordered_map<std::string, Content *>();

        return contents->at(ordinal);
    }

    Mix_Chunk *Contents::loadSound(const std::string &path) {
        std::string actual("assets/sounds/");
        actual.append(path).append(".ogg");

        Mix_Chunk *chunk = Mix_LoadWAV(actual.c_str());
        if(!chunk) throw std::runtime_error(std::string("Couldn't load '").append(actual).append("': ").append(Mix_GetError()).c_str());

        return chunk;
    }

    Content::Content(const std::string &name) {
        this->name = name;
    }

    EntityType::EntityType(const std::string &name, const std::function<void(entt::entity)> &initializer): Content(name) {
        this->initializer = initializer;
    }

    entt::entity EntityType::create() {
        entt::entity e = App::iregistry().create();
        initializer(e);

        return e;
    }

    CType EntityType::ctype() {
        return CType::ENTITY;
    }

    EffectType::EffectType(const std::string &name, DrawType *drawer): EffectType(name, drawer->name) {}
    EffectType::EffectType(const std::string &name, const std::string &drawer): EffectType(name,
        [this](entt::entity e) {
            entt::registry &registry = App::iregistry();

            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.gravityScale = 0.0f;
            bodyDef.position.SetZero();

            b2PolygonShape shape;
            shape.SetAsBox(clipSize / 2.0f, clipSize / 2.0f);

            b2FixtureDef fixt;
            fixt.shape = &shape;
            fixt.isSensor = true;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            registry.emplace<RigidComp>(e, e, body);
            registry.emplace<TemporalComp>(e, e, TemporalComp::TIME).time = lifetime;
            registry.emplace<DrawComp>(e, e, this->drawer).z = z;
        }, drawer
    ) {}
    
    EffectType::EffectType(const std::string &name, const std::function<void(entt::entity)> &initializer, DrawType *drawer): EffectType(name, initializer, drawer->name) {}
    EffectType::EffectType(const std::string &name, const std::function<void(entt::entity)> &initializer, const std::string &drawer): EntityType(name, initializer) {
        this->drawer = drawer;
        clipSize = 1.0f;
        lifetime = 1.0f;
        z = 1.0f;
    }

    CType EffectType::ctype() {
        return CType::EFFECT;
    }

    DrawType::DrawType(const std::string &name, const std::function<void(entt::entity)> &drawer): Content(name) {
        this->drawer = drawer;
    }

    CType DrawType::ctype() {
        return CType::DRAW;
    }
}
