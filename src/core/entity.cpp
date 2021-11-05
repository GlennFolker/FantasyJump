#include <glm/gtx/transform.hpp>

#include "entity.h"
#include "time.h"
#include "../app.h"
#include "../util/mathf.h"

namespace Fantasy {
    Component::Component(entt::entity e) {
        ref = e;
    }

    void Component::update() {}

    entt::entity Component::getRef() {
        return ref;
    }

    entt::registry &Component::getRegistry() {
        return *App::instance->control->regist;
    }

    b2World &Component::getWorld() {
        return *App::instance->control->world;
    }

    RigidComp::RigidComp(entt::entity e, b2Body *body): Component(e) {
        this->body = body;
        this->body->GetUserData().pointer = (uintptr_t)ref;
        rotateSpeed = 0.0f;
    }

    void RigidComp::update() {
        Component::update();
        if(!Mathf::near(rotateSpeed, 0.0f)) body->SetTransform(body->GetPosition(), body->GetAngle() + rotateSpeed);
    }

    void RigidComp::beginCollide(RigidComp &other) {
        entt::registry &registry = getRegistry();
        if(registry.any_of<HealthComp>(ref) && registry.any_of<HealthComp>(other.ref)) {
            HealthComp &self = registry.get<HealthComp>(ref);
            HealthComp &otherh = registry.get<HealthComp>(other.ref);

            if(!Mathf::near(otherh.getDamage(), 0.0f) && self.canHurt()) self.hurt(otherh.getDamage());
            if(!Mathf::near(self.getDamage(), 0.0f) && otherh.canHurt()) otherh.hurt(self.getDamage());
        }
    }

    void RigidComp::endCollide(RigidComp &other) {

    }

    void RigidComp::onDestroy(entt::registry &registry, entt::entity entity) {
        RigidComp &comp = registry.get<RigidComp>(entity);
        comp.getWorld().DestroyBody(comp.body);
    }

    SpriteComp::SpriteComp(entt::entity e, Tex2D *texture): SpriteComp(e, texture, 1.0f, 1.0f) {}
    SpriteComp::SpriteComp(entt::entity e, Tex2D *texture, float size): SpriteComp(e, texture, size, size) {}
    SpriteComp::SpriteComp(entt::entity e, Tex2D *texture, float width, float height): Component(e) {
        this->texture = texture;
        this->width = width;
        this->height = height;
    }

    void SpriteComp::update() {
        Component::update();

        b2Transform trns = getRegistry().get<RigidComp>(ref).body->GetTransform();
        App::instance->renderer->batch->draw(texture, trns.p.x, trns.p.y, width, height, degrees(trns.q.GetAngle()));
    }

    JumpComp::JumpComp(entt::entity e, float force, float timeout): Component(e) {
        this->force = force;
        this->timeout = timeout;
        holding = false;
        jumping = false;
        time = -1.0f;
    }

    void JumpComp::hold() {
        holding = true;
        jumping = false;
        time = Time::time();
    }

    void JumpComp::release(float x, float y) {
        holding = false;
        jumping = true;
        target = b2Vec2(x, y);
    }

    void JumpComp::update() {
        Component::update();

        b2Body *body = getRegistry().get<RigidComp>(ref).body;
        float now = Time::time();

        if(holding) {
            b2Vec2 vel = body->GetLinearVelocity();
            vel *= (-1.0f * fminf((now - time) / 0.1f, 1.0f)) * vel.Length();

            body->ApplyForceToCenter(vel, true);
            if(!Mathf::near(body->GetAngularVelocity(), 1.0f)) {
                body->ApplyAngularImpulse(body->GetAngularVelocity() * -0.01f, true);
            }
        } else if(jumping) {
            jumping = false;

            b2Vec2 impulse = target - body->GetPosition();
            impulse.Normalize();
            impulse *= force * fminf((now - time) / timeout, 1.0f);

            body->ApplyLinearImpulse(impulse, b2Vec2(
                body->GetPosition().x + Mathf::random(-0.1f, 0.1f),
                body->GetPosition().y + Mathf::random(-0.1f, 0.1f)
            ), true);
        }
    }

    HealthComp::HealthComp(entt::entity e, float health): HealthComp(e, health, 0.0f) {}
    HealthComp::HealthComp(entt::entity e, float health, float damage): Component(e) {
        this->health = maxHealth = health;
        this->damage = damage;
    }

    void HealthComp::update() {
        if(canHurt() && health <= 0.0f) killed();
    }

    void HealthComp::kill() {
        health = 0.0f;
    }

    void HealthComp::killed() {
        App::instance->control->scheduleRemoval(ref);
    }

    void HealthComp::heal(float heal) {
        if(canHurt()) health = fminf(health + heal, maxHealth);
    }

    void HealthComp::hurt(float damage) {
        if(canHurt()) health = fmaxf(health - damage, 0.0f);
    }

    bool HealthComp::canHurt() {
        return health != -1.0f;
    }

    float HealthComp::getHealth() {
        return health;
    }

    float HealthComp::getDamage() {
        return damage;
    }
}
