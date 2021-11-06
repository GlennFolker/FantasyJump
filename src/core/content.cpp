#include <glm/gtx/transform.hpp>

#include "content.h"
#include "entity.h"
#include "../util/mathf.h"

namespace Fantasy {
    Contents::Contents() {
        contents = new std::vector<std::unordered_map<const char *, Content *> *>((int)CType::ALL);

        jumpTexture = new Tex2D("assets/jumper.png");
        jumpTexture->load();
        jumpTexture->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);

        spikeTexture = new Tex2D("assets/spike.png");
        spikeTexture->load();
        spikeTexture->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);

        jumper = create<EntityType>("jumper", [&](entt::registry &registry, b2World &world, entt::entity e) {
            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();

            b2CircleShape shape;
            shape.m_radius = 0.5f;

            b2FixtureDef fixt;
            fixt.shape = &shape;
            fixt.density = 1.0f;

            b2Body *body = world.CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            registry.emplace<RigidComp>(e, e, body);
            registry.emplace<SpriteComp>(e, e, jumpTexture);
            registry.emplace<JumpComp>(e, e, 20.0f, 0.7f);
            registry.emplace<HealthComp>(e, e, 100.0f);
        });

        spike = create<EntityType>("spike", [&](entt::registry &registry, b2World &world, entt::entity e) {
            b2BodyDef bodyDef;
            bodyDef.type = b2_kinematicBody;
            bodyDef.position.SetZero();

            b2CircleShape shape;
            shape.m_radius = 0.9f;

            b2FixtureDef fixt;
            fixt.restitution = 1.6f;
            fixt.restitutionThreshold = 0.0f;
            fixt.shape = &shape;

            b2Body *body = world.CreateBody(&bodyDef);
            body->CreateFixture(&fixt);
            
            registry.emplace<RigidComp>(e, e, body).rotateSpeed = glm::radians(Mathf::random() > 0.5f ? 1.0f : -1.0f);
            registry.emplace<SpriteComp>(e, e, spikeTexture, 2.0f);
            registry.emplace<HealthComp>(e, e, -1.0f, 10.0f);
        });
    }

    Contents::~Contents() {
        for(auto arr : *contents) delete arr;
        delete contents;
        delete jumpTexture;
        delete spikeTexture;
    }

    std::unordered_map<const char *, Content *> *Contents::getBy(CType type) {
        int ordinal = (int)type;
        if(contents->at(ordinal) == NULL) contents->insert(contents->begin() + ordinal, new std::unordered_map<const char *, Content *>());

        return contents->at(ordinal);
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
