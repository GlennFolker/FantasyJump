#include <glm/gtx/transform.hpp>

#include "content.h"
#include "entity.h"
#include "../util/mathf.h"
#include "../app.h"

namespace Fantasy {
    Contents::Contents() {
        contents = new std::vector<std::unordered_map<std::string, Content *> *>((int)CType::ALL);

        genericRegion = create<DrawType>("drawer-generic-region", [](entt::entity e) {
            entt::registry &registry = App::iregistry();

            DrawComp &comp = registry.get<DrawComp>(e);
            if(!comp.region) return;
            
            b2Body *body = registry.get<RigidComp>(e).body;
            b2Vec2 pos = body->GetPosition();
            App::ibatch().draw(*comp.region, pos.x, pos.y, comp.width, comp.height, body->GetAngle());
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
            fixt.density = 1.0f;
            fixt.friction = 0.1f;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            registry.emplace<RigidComp>(e, e, body).deathFx = destructMed->name;
            registry.emplace<DrawComp>(e, e, genericRegion->name, 1.0f, 1.0f, 2.0f).region = App::iatlas().get("jumper");
            registry.emplace<JumpComp>(e, e, 20.0f, 0.7f);
            registry.emplace<HealthComp>(e, e, 100.0f, 5.0f);
            registry.emplace<TeamComp>(e, e, Team::BLUE);
            registry.emplace<ShooterComp>(e, e, bulletSmall->name, 0.3f).shootFx = smokeSmall->name;
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
            fixt.density = 2.0f;
            fixt.friction = 0.3f;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);
            
            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.rotateSpeed = glm::radians(Mathf::random(1.0f, 2.5f) * (Mathf::random() >= 0.5f ? 1.0f : -1.0f));
            comp.deathFx = destructBig->name;

            registry.emplace<DrawComp>(e, e, genericRegion->name, 2.0f, 2.0f, 1.0f).region = App::iatlas().get("spike");
            registry.emplace<HealthComp>(e, e, 100.0f, 10.0f);
            registry.emplace<TeamComp>(e, e, Team::GENERIC);
            registry.emplace<ShooterComp>(e, e, bulletMed->name, 1.2f, 5.0f, 10.0f).shootFx = smokeBig->name;
        });

        leak = create<EntityType>("ent-leak", [](entt::entity e) {

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

            registry.emplace<RigidComp>(e, e, body).deathFx = destructSmall->name;
            registry.emplace<DrawComp>(e, e, genericRegion->name, 0.5f, 0.5f, 3.0f).region = App::iatlas().get("bullet-small");
            registry.emplace<HealthComp>(e, e, 5.0f, 10.0f).selfDamage = true;
            registry.emplace<TeamComp>(e, e);
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
            fixt.friction = 0.08f;

            b2Body *body = App::iworld().CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            RigidComp &comp = registry.emplace_or_replace<RigidComp>(e, e, body);
            comp.rotateSpeed = glm::radians(Mathf::random() > 0.5f ? 10.0f : -10.0f);
            comp.deathFx = destructSmall->name;

            registry.emplace<DrawComp>(e, e, genericRegion->name, 0.75f, 0.75f, 3.0f).region = App::iatlas().get("bullet-medium");
            registry.emplace<HealthComp>(e, e, 10.0f, 20.0f).selfDamage = true;
            registry.emplace<TeamComp>(e, e);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });

        smokeSmall = create<EffectType>("fx-smoke-small", create<DrawType>("fx-smoke-small-drawer", [](entt::entity e) {
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
        }));
        smokeSmall->lifetime = 0.24f;
        smokeSmall->z = 5.0f;

        smokeBig = create<EffectType>("fx-smoke-big", create<DrawType>("fx-smoke-big-drawer", [](entt::entity e) {
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
        }));
        smokeBig->lifetime = 0.4f;
        smokeBig->z = 6.0f;

        destructSmall = create<EffectType>("fx-destruct-small", create<DrawType>("fx-destruct-small-drawer", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();
            SpriteBatch &batch = App::ibatch();

            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);
            b2Vec2 pos = comp.body->GetPosition();

            float l = life.timef();
            l = (1.0f - powf(1.0f - l, 2.0f));

            batch.col(Color(Color::lyellow).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 6, 1.2f, l, [&](float x, float y) {
                float s = (1.0f - l) * 0.4f;
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, s, s);
            });

            float size = l * 1.2f;
            batch.col(Color(Color::yellow).lerp(Color(), l));
            batch.draw(atlas.get("white"), pos.x, pos.y, size, size);
        }));
        destructSmall->lifetime = 0.24f;
        destructSmall->z = 8.0f;

        destructMed = create<EffectType>("fx-destruct-medium", create<DrawType>("fx-destruct-medium-drawer", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();
            SpriteBatch &batch = App::ibatch();

            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);
            b2Vec2 pos = comp.body->GetPosition();

            float l = life.timef();
            l = (1.0f - powf(1.0f - l, 2.5f));

            App::ibatch().col(Color(Color::lorange).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 12, 4.0f, l, [&](float x, float y) {
                float s = (1.0f - l) * 1.3f;
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, s, s);
            });

            float size = l * 5.0f;
            batch.col(Color(Color::orange).lerp(Color(), l));
            batch.draw(atlas.get("white"), pos.x, pos.y, size, size);
        }));
        destructMed->lifetime = 0.8f;
        destructMed->z = 8.0f;

        destructBig = create<EffectType>("fx-destruct-big", create<DrawType>("fx-destruct-big-drawer", [](entt::entity e) {
            entt::registry &registry = App::iregistry();
            TexAtlas &atlas = App::iatlas();
            SpriteBatch &batch = App::ibatch();

            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);
            b2Vec2 pos = comp.body->GetPosition();

            float l = life.timef();
            l = (1.0f - powf(1.0f - l, 3.0f));

            batch.col(Color(Color::lred).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 17, 7.0f, l, [&](float x, float y) {
                float s = (1.0f - l) * 2.0f;
                batch.draw(atlas.get("white"), pos.x + x, pos.y + y, s, s);
            });

            float size = l * 7.0f;
            batch.col(Color(Color::red).lerp(Color(), l));
            batch.draw(atlas.get("white"), pos.x, pos.y, size, size);
        }));
        destructBig->lifetime = 1.5f;
        destructBig->z = 9.0f;
    }

    Contents::~Contents() {
        delete contents;
    }

    std::unordered_map<std::string, Content *> *Contents::getBy(CType type) {
        int ordinal = (int)type;
        if(contents->at(ordinal) == NULL) contents->at(ordinal) = new std::unordered_map<std::string, Content *>();

        return contents->at(ordinal);
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
