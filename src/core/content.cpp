#include "content.h"
#include "entity.h"
#include "../app.h"

namespace Fantasy {
    Tex2D *texture = new Tex2D("assets/texture.png");

    Contents::Contents() {
        contents = new std::vector<std::unordered_map<const char *, Content *> *>((int)CType::ALL);
        texture->load();

        jumper = create<EntityType>("jumper", [&](entt::registry &registry, b2World &world, entt::entity e) {
            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.SetZero();

            b2PolygonShape shape;
            shape.SetAsBox(0.5f, 0.5f);

            b2FixtureDef fixt;
            fixt.shape = &shape;
            fixt.density = 1.0f;

            b2Body *body = world.CreateBody(&bodyDef);
            body->CreateFixture(&fixt);

            registry.emplace<RigidComp>(e, e, body);
            registry.emplace<SpriteComp>(e, e, texture);
            registry.emplace<JumpComp>(e, e, 20.0f, 1.0f);
        });
    }

    Contents::~Contents() {
        for(auto arr : *contents) delete arr;
        delete contents;
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
