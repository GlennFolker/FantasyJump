#include <glm/gtx/transform.hpp>

#include "entity.h"
#include "events.h"
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
        if(!Mathf::near(rotateSpeed, 0.0f)) body->SetTransform(body->GetPosition(), body->GetAngle() + rotateSpeed);
    }

    void RigidComp::beginCollide(RigidComp &other) {
        entt::registry &registry = getRegistry();
        if(registry.any_of<HealthComp>(ref) && registry.any_of<HealthComp>(other.ref)) {
            HealthComp &self = registry.get<HealthComp>(ref);
            HealthComp &otherh = registry.get<HealthComp>(other.ref);

            if(!Mathf::near(otherh.damage, 0.0f) && self.canHurt()) {
                self.hurt(otherh.damage);
                if(otherh.selfDamage) otherh.hurt(otherh.damage);
            }
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
    SpriteComp::SpriteComp(entt::entity e, Tex2D *texture, float width, float height): SpriteComp(e, texture, width, height, 0.0f) {}
    SpriteComp::SpriteComp(entt::entity e, Tex2D *texture, float width, float height, float z): Component(e) {
        this->texture = texture;
        this->width = width;
        this->height = height;
        this->z = z;
    }

    void SpriteComp::update() {
        entt::registry &registry = getRegistry();
        b2Transform trns = registry.get<RigidComp>(ref).body->GetTransform();
        SpriteBatch *batch = App::instance->renderer->batch;

        if(registry.any_of<HealthComp>(ref)) {
            float alpha = fmaxf(1.0f - (Time::time() - registry.get<HealthComp>(ref).hitTime) / 0.5f, 0.0f);
            batch->tint(Color(1.0f, 0.0f, 0.3f, alpha));
        }

        batch->z = z;
        batch->draw(texture, trns.p.x, trns.p.y, width, height, degrees(trns.q.GetAngle()));
        batch->tint(Color());
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
        hitTime = -1000.0f;
        selfDamage = false;
    }

    void HealthComp::update() {
        if(canHurt() && health <= 0.0f) killed();
    }

    void HealthComp::kill() {
        health = 0.0f;
    }

    void HealthComp::killed() {
        Events::fire<EntDeathEvent>(EntDeathEvent(ref));
        App::instance->control->scheduleRemoval(ref);
    }

    void HealthComp::heal(float heal) {
        if(canHurt()) health = fminf(health + heal, maxHealth);
    }

    void HealthComp::hurt(float damage) {
        if(canHurt()) {
            float prev = health;
            health = fmaxf(health - damage, 0.0f);

            if(!Mathf::near(prev, health)) hitTime = Time::time();
        }
    }

    bool HealthComp::canHurt() {
        return health != -1.0f;
    }
}
