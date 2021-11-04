#include <glm/gtx/transform.hpp>

#include "entity.h"
#include "time.h"
#include "../app.h"

namespace Fantasy {
    Component::Component(entt::entity e) {
        ref = e;
    }

    RigidComp::RigidComp(entt::entity e, b2Body *body): Component(e) {
        this->body = body;
    }

    void RigidComp::update() {}

    void RigidComp::onDestroy(entt::registry &registry, entt::entity entity) {
        RigidComp &comp = registry.get<RigidComp>(entity);
        App::instance->control->world->DestroyBody(comp.body);
    }

    SpriteComp::SpriteComp(entt::entity e, Tex2D *texture): Component(e) {
        this->texture = texture;
    }

    void SpriteComp::update() {
        b2Transform trns = App::instance->control->regist->get<RigidComp>(ref).body->GetTransform();
        App::instance->renderer->batch->draw(texture, trns.p.x, trns.p.y, 1.0f, 1.0f, degrees(trns.q.GetAngle()));
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
        b2Body *body = App::instance->control->regist->get<RigidComp>(ref).body;
        float now = Time::time();

        if(holding) {
            b2Vec2 vel = body->GetLinearVelocity();
            vel *= -1.0f * vel.Length();

            body->ApplyForceToCenter(vel, true);
        } else if(jumping) {
            jumping = false;

            b2Vec2 impulse = target - body->GetPosition();
            impulse.Normalize();
            impulse *= force * fminf(0.3f + ((now - time) / timeout) * 0.7f, 1.0f);

            body->ApplyLinearImpulseToCenter(impulse, true);
        }
    }
}
