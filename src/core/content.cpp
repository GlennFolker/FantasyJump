#include <glm/gtx/transform.hpp>

#include "content.h"
#include "entity.h"
#include "../util/mathf.h"
#include "../app.h"

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

            registry.emplace<RigidComp>(e, e, body).deathFx = destructMed->name;
            registry.emplace<SpriteComp>(e, e, atlas.get("jumper"), 1.0f, 1.0f, 2.0f);
            registry.emplace<JumpComp>(e, e, 20.0f, 0.7f);
            registry.emplace<HealthComp>(e, e, 100.0f, 5.0f);
            registry.emplace<TeamComp>(e, e, Team::BLUE);
            registry.emplace<ShooterComp>(e, e, bulletSmall->name, 0.3f).shootFx = smokeSmall->name;
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
            
            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.rotateSpeed = glm::radians(Mathf::random(1.0f, 2.5f) * (Mathf::random() >= 0.5f ? 1.0f : -1.0f));
            comp.deathFx = destructBig->name;

            registry.emplace<SpriteComp>(e, e, atlas.get("spike"), 2.0f, 2.0f, 1.0f);
            registry.emplace<HealthComp>(e, e, 100.0f, 10.0f);
            registry.emplace<TeamComp>(e, e, Team::GENERIC);
            registry.emplace<ShooterComp>(e, e, bulletMed->name, 1.2f, 5.0f, 10.0f).shootFx = smokeBig->name;
        });

        bulletSmall = create<EntityType>("bullet-small", [this](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
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

            RigidComp &comp = registry.emplace<RigidComp>(e, e, body);
            comp.deathFx = destructSmall->name;

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

            RigidComp &comp = registry.emplace_or_replace<RigidComp>(e, e, body);
            comp.rotateSpeed = glm::radians(Mathf::random() > 0.5f ? 10.0f : -10.0f);
            comp.deathFx = destructSmall->name;

            registry.emplace<SpriteComp>(e, e, atlas.get("bullet-medium"), 0.75f, 0.75f, 3.0f);
            registry.emplace<HealthComp>(e, e, 10.0f, 20.0f).selfDamage = true;
            registry.emplace<TeamComp>(e, e);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });

        smokeSmall = create<EffectType>("smoke-small", [](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);

            float l = life.timef();
            App::ibatch().col(Color(Color::lyellow).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 3, 1.2f, l, [&](float x, float y) {
                float s = (1.0f - powf(l, 2.0f)) * 0.32f;
                App::ibatch().draw(atlas.get("white"), comp.body->GetPosition().x + x, comp.body->GetPosition().y + y, s, s, glm::radians(45.0f));
            });
        });
        smokeSmall->lifetime = 0.24f;
        smokeSmall->z = 5.0f;

        smokeBig = create<EffectType>("smoke-big", [](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);

            float l = life.timef();
            App::ibatch().col(Color(Color::lyellow).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 4, 2.0f, l, [&](float x, float y) {
                float s = (1.0f - powf(l, 3.0f)) * 0.5f;
                App::ibatch().draw(atlas.get("white"), comp.body->GetPosition().x + x, comp.body->GetPosition().y + y, s, s, glm::radians(45.0f));
            });
        });
        smokeBig->lifetime = 0.4f;
        smokeBig->z = 6.0f;

        destructSmall = create<EffectType>("destruct-small", [](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);

            float l = life.timef();
            l = (1.0f - powf(1.0f - l, 2.0f));

            App::ibatch().col(Color(Color::lyellow).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 6, 1.2f, l, [&](float x, float y) {
                float s = (1.0f - l) * 0.4f;
                App::ibatch().draw(atlas.get("white"), comp.body->GetPosition().x + x, comp.body->GetPosition().y + y, s, s);
            });

            float size = l * 1.2f;
            App::ibatch().col(Color(Color::yellow).lerp(Color(), l));
            App::ibatch().draw(atlas.get("white"), comp.body->GetPosition().x, comp.body->GetPosition().y, size, size);
        });
        destructSmall->lifetime = 0.24f;
        destructSmall->z = 8.0f;

        destructMed = create<EffectType>("destruct-medium", [](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);

            float l = life.timef();
            l = (1.0f - powf(1.0f - l, 2.5f));

            App::ibatch().col(Color(Color::lorange).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 12, 4.0f, l, [&](float x, float y) {
                float s = (1.0f - l) * 1.3f;
                App::ibatch().draw(atlas.get("white"), comp.body->GetPosition().x + x, comp.body->GetPosition().y + y, s, s);
            });

            float size = l * 5.0f;
            App::ibatch().col(Color(Color::orange).lerp(Color(), l));
            App::ibatch().draw(atlas.get("white"), comp.body->GetPosition().x, comp.body->GetPosition().y, size, size);
        });
        destructMed->lifetime = 0.8f;
        destructMed->z = 8.0f;

        destructBig = create<EffectType>("destruct-big", [](entt::registry &registry, const TexAtlas &atlas, b2World &world, entt::entity e) {
            RigidComp &comp = registry.get<RigidComp>(e);
            TemporalComp &life = registry.get<TemporalComp>(e);

            float l = life.timef();
            l = (1.0f - powf(1.0f - l, 3.0f));

            App::ibatch().col(Color(Color::lred).lerp(Color::gray, l));
            Mathf::randVecs((unsigned int)e, 17, 7.0f, l, [&](float x, float y) {
                float s = (1.0f - l) * 2.0f;
                App::ibatch().draw(atlas.get("white"), comp.body->GetPosition().x + x, comp.body->GetPosition().y + y, s, s);
            });

            float size = l * 7.0f;
            App::ibatch().col(Color(Color::red).lerp(Color(), l));
            App::ibatch().draw(atlas.get("white"), comp.body->GetPosition().x, comp.body->GetPosition().y, size, size);
        });
        destructBig->lifetime = 1.5f;
        destructBig->z = 9.0f;
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
            bodyDef.type = b2_dynamicBody;
            bodyDef.gravityScale = 0.0f;
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
            EffectComp &comp = registry.emplace<EffectComp>(e, e);
            comp.effect = this->name;
            comp.z = z;
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
