#include <glm/gtx/transform.hpp>

#include "content.h"
#include "entity.h"
#include "../util/mathf.h"

namespace Fantasy {
    Contents::Contents() {
        contents = new std::vector<std::unordered_map<const char *, Content *> *>((int)CType::ALL);

        jumpTexture = loadTex("assets/jumper.png");
        spikeTexture = loadTex("assets/spike.png");
        bulletSmallTexture = loadTex("assets/bullet-small.png");
        bulletMedTexture = loadTex("assets/bullet-medium.png");

        jumper = create<EntityType>("jumper", [&](entt::registry &registry, b2World &world, entt::entity e) {
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
            registry.emplace<SpriteComp>(e, e, jumpTexture, 1.0f, 1.0f, 2.0f);
            registry.emplace<JumpComp>(e, e, 20.0f, 0.7f);
            registry.emplace<HealthComp>(e, e, 100.0f, 5.0f);
            registry.emplace<TeamComp>(e, e, Team::BLUE);
            registry.emplace<ShooterComp>(e, e, ShooterComp::SMALL, 0.3f);
        });

        spike = create<EntityType>("spike", [&](entt::registry &registry, b2World &world, entt::entity e) {
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
            registry.emplace<SpriteComp>(e, e, spikeTexture, 2.0f, 2.0f, 1.0f);
            registry.emplace<HealthComp>(e, e, 100.0f, 10.0f);
            registry.emplace<TeamComp>(e, e, Team::GENERIC);
            registry.emplace<ShooterComp>(e, e, ShooterComp::MEDIUM, 1.2f, 5.0f, 10.0f);
        });

        bulletSmall = create<EntityType>("bullet-small", [&](entt::registry &registry, b2World &world, entt::entity e) {
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
            registry.emplace<SpriteComp>(e, e, bulletSmallTexture, 0.5f, 0.5f, 3.0f);
            registry.emplace<HealthComp>(e, e, 5.0f, 10.0f).selfDamage = true;
            registry.emplace<TeamComp>(e, e);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });

        bulletMed = create<EntityType>("bullet-medium", [&](entt::registry &registry, b2World &world, entt::entity e) {
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

            registry.emplace<RigidComp>(e, e, body).rotateSpeed = glm::radians(Mathf::random() > 0.5f ? 10.0f : -10.0f);
            registry.emplace<SpriteComp>(e, e, bulletMedTexture, 0.75f, 0.75f, 3.0f);
            registry.emplace<HealthComp>(e, e, 10.0f, 20.0f).selfDamage = true;
            registry.emplace<TeamComp>(e, e);
            registry.emplace<TemporalComp>(e, e, TemporalComp::RANGE);
        });
    }

    Contents::~Contents() {
        delete contents;
        delete jumpTexture;
        delete spikeTexture;
        delete bulletSmallTexture;
        delete bulletMedTexture;
    }

    std::unordered_map<const char *, Content *> *Contents::getBy(CType type) {
        int ordinal = (int)type;
        if(contents->at(ordinal) == NULL) contents->insert(contents->begin() + ordinal, new std::unordered_map<const char *, Content *>());

        return contents->at(ordinal);
    }

    Tex2D *Contents::loadTex(const char *name) {
        Tex2D *tex = new Tex2D(name);
        tex->load();
        tex->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
        return tex;
    }

    Content::Content(const char *name) {
        this->name = name;
    }

    EntityType::EntityType(const char *name, std::function<void(entt::registry &, b2World &, entt::entity)> &&initializer): Content(name) {
        this->initializer = initializer;
    }

    entt::entity EntityType::create(entt::registry &registry, b2World &world) {
        entt::entity e = registry.create();
        initializer(registry, world, e);

        return e;
    }

    CType EntityType::ctype() {
        return CType::ENTITY;
    }
}
