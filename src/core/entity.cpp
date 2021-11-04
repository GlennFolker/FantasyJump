#include <glm/gtx/transform.hpp>

#include "entity.h"
#include "time.h"
#include "../app.h"
#include "../util/mathf.h"

namespace Fantasy {
    Component::Component(entt::entity e) {
        ref = e;
        listener = [](entt::entity) {};
    }

    void Component::update() {
        listener(ref);
    }

    RigidComp::RigidComp(entt::entity e, b2Body *body): Component(e) {
        this->body = body;
        this->body->GetUserData().pointer = (uintptr_t)this;
    }

    void RigidComp::update() {
        Component::update();
    }

    void RigidComp::onDestroy(entt::registry &registry, entt::entity entity) {
        RigidComp &comp = registry.get<RigidComp>(entity);
        App::instance->control->world->DestroyBody(comp.body);
    }

    SpriteComp::SpriteComp(entt::entity e, Tex2D *texture, float width, float height): Component(e) {
        this->texture = texture;
        this->width = width;
        this->height = height;
    }

    void SpriteComp::update() {
        Component::update();

        b2Transform trns = App::instance->control->regist->get<RigidComp>(ref).body->GetTransform();
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

        b2Body *body = App::instance->control->regist->get<RigidComp>(ref).body;
        float now = Time::time();

        if(holding) {
            b2Vec2 vel = body->GetLinearVelocity();
            vel *= (-1.0f * fminf((now - time) / 0.3f, 1.0f)) * vel.Length();

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
}
