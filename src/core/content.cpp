#include "content.h"
#include "entity.h"
#include "../util/mathf.h"
#include "../app.h"

#include <glm/gtx/transform.hpp>

namespace Fantasy {
    Contents::Contents() {
        contents = new std::vector<std::unordered_map<std::string, Content *> *>((int)CType::ALL);

        jumper = create<EntityType>("jumper", [this](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();

            b2CircleShape shape;
            shape.m_radius = 0.5f;

            b2FixtureDef fixt;
            fixt.shape = &shape;
            fixt.density = 1.0f;
            fixt.friction = 0.1f;

            b2Body *body = world.CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            registry.emplace<RigidComp>(e, e, body);
            registry.emplace<SpriteComp>(e, e, atlas.get("jumper"), 1.0f, 1.0f, 2.0f);
            registry.emplace<JumpComp>(e, e, 20.0f, 0.7f);
            registry.emplace<HealthComp>(e, e, 100.0f, 5.0f);
            registry.emplace<TeamComp>(e, e, Team::BLUE);
            registry.emplace<ShooterComp>(e, e, bulletSmall->name, 0.3f);
        });

        spike = create<EntityType>("spike", [this](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
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

            b2Body *body = world.CreateBody(&bodyDef);
            body->CreateFixture(&fixt);
            
            registry.emplace<RigidComp>(e, e, body).rotateSpeed = glm::radians(Mathf::random(1.0f, 2.5f) * (Mathf::random() >= 0.5f ? 1.0f : -1.0f));
            registry.emplace<SpriteComp>(e, e, atlas.get("spike"), 2.0f, 2.0f, 1.0f);
            registry.emplace<HealthComp>(e, e, 100.0f, 10.0f);
            registry.emplace<TeamComp>(e, e, Team::GENERIC);
            registry.emplace<ShooterComp>(e, e, bulletMed->name, 1.2f, 5.0f, 10.0f);
        });

        bulletSmall = create<EntityType>("bullet-small", [](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
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

            b2Body *body = world.CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            registry.emplace<RigidComp>(e, e, body);
            registry.emplace<SpriteComp>(e, e, atlas.get("bullet-small"), 0.5f, 0.5f, 3.0f);
            registry.emplace<HealthComp>(e, e, 5.0f, 10.0f).selfDamage = true;
            registry.emplace<TeamComp>(e, e);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });

        bulletMed = create<EntityType>("bullet-medium", [this](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
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

            b2Body *body = world.CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.rotateSpeed = glm::radians(Mathf::random() > 0.5f ? 10.0f : -10.0f);
            comp.spawnFx = fx->name;

            registry.emplace<SpriteComp>(e, e, atlas.get("bullet-medium"), 0.75f, 0.75f, 3.0f);
            registry.emplace<HealthComp>(e, e, 10.0f, 20.0f).selfDamage = true;
            registry.emplace<TeamComp>(e, e);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });

        fx = create<EffectType>("effect", [](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);

            float s = 1.0f - powf(life.timef(), 3.0f);
            App::instance->renderer->batch->draw(atlas.get("white"), comp.body->GetPosition().x, comp.body->GetPosition().y, s, s);
        });
        fx->z = 5.0f;
    }

    Contents::~Contents() {
        delete contents;
    }

    std::unordered_map<std::string, Content *> *Contents::getBy(CType type) {
        int ordinal = (int)type;
        if(contents->at(ordinal) == NULL) contents->insert(contents->begin() + ordinal, new std::unordered_map<std::string, Content *>());

        return contents->at(ordinal);
    }

    Content::Content(const std::string &name) {
        this->name = name;
    }

    EntityType::EntityType(const std::string &name, const std::function<void(entt::registry &, const TexAtlas &, b2World &, entt::entity)> &initializer): Content(name) {
        this->initializer = initializer;
    }

    entt::entity EntityType::create(entt::registry &registry, const TexAtlas &atlas, b2World &world) {
        entt::entity e = registry.create();
        initializer(registry, atlas, world, e);

        return e;
    }

    CType EntityType::ctype() {
        return CType::ENTITY;
    }

    EffectType::EffectType(const std::string &name, const std::function<void(entt::registry &, const TexAtlas &, b2World &, entt::entity)> &updater): EffectType(name,
        [this](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
            b2BodyDef bodyDef;
            bodyDef.type = b2_kinematicBody;
            bodyDef.position.SetZero();

            b2PolygonShape shape;
            shape.SetAsBox(clipSize / 2.0f, clipSize / 2.0f);

            b2FixtureDef fixt;
            fixt.shape = &shape;
            fixt.isSensor = true;

            b2Body *body = world.CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            registry.emplace<RigidComp>(e, e, body);
            registry.emplace<TemporalComp>(e, e, TemporalComp::TIME).time = lifetime;
            registry.emplace<EffectComp>(e, e).z = z;
        }, updater
    ) {}
    EffectType::EffectType(const std::string &name,
        const std::function<void(entt::registry &, const TexAtlas &, b2World &, entt::entity)> &initializer,
        const std::function<void(entt::registry &, const TexAtlas &, b2World &, entt::entity)> &updater
    ): EntityType(name, initializer) {
        this->updater = updater;
        clipSize = 1.0f;
        lifetime = 1.0f;
        z = 1.0f;
    }

    CType EffectType::ctype() {
        return CType::EFFECT;
    }
}
