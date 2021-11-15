#include "entity.h"
#include "events.h"
#include "time.h"
#include "../app.h"
#include "../util/mathf.h"

#include <glm/gtx/vector_angle.hpp>

namespace Fantasy {
    Component::Component(entt::entity e) {
        ref = e;
    }

    void Component::update() {}
    void Component::remove() {
        App::instance->control->scheduleRemoval(ref);
    }

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

            if(otherh.damage > 0.0f && self.canHurt()) {
                self.hurt(otherh.damage);
                if(otherh.selfDamage) otherh.hurt(otherh.damage);
            }
        }
    }

    void RigidComp::endCollide(RigidComp &other) {}

    bool RigidComp::shouldCollide(RigidComp &other) {
        entt::registry &regist = getRegistry();
        if(!regist.any_of<TeamComp>(ref) || !regist.any_of<TeamComp>(other.ref)) {
            return true;
        } else {
            return regist.get<TeamComp>(ref).team != regist.get<TeamComp>(other.ref).team;
        }
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
            batch->tint(Color(0.8f, 0.0f, 0.1f, alpha));
        }

        batch->z = z;
        batch->draw(texture, trns.p.x, trns.p.y, width, height, trns.q.GetAngle());
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
        remove();
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

    TeamComp::TeamComp(entt::entity e): TeamComp(e, Team::GENERIC) {}
    TeamComp::TeamComp(entt::entity e, Team::TeamType team): Component(e) {
        this->team = team;
    }

    ShooterComp::ShooterComp(entt::entity e, ShootType type, float rate): ShooterComp(e, type, rate, 4.0f) {}
    ShooterComp::ShooterComp(entt::entity e, ShootType type, float rate, float impulse): ShooterComp(e, type, rate, impulse, 20.0f) {}
    ShooterComp::ShooterComp(entt::entity e, ShootType type, float rate, float impulse, float range): Component(e) {
        this->type = type;
        this->rate = rate;
        this->impulse = impulse;
        this->range = range;
        time = -rate;
    }

    void ShooterComp::update() {
        entt::registry &regist = getRegistry();
        b2World &world = getWorld();
        b2Body *body = regist.get<RigidComp>(ref).body;

        if((Time::time() - time) >= rate) {
            b2Vec2 pos = body->GetPosition();

            class Report: public b2QueryCallback {
                private:
                b2Vec2 origin;
                float radius;
                Team::TeamType team;

                b2Body *closest;

                public:
                Report(b2Vec2 origin, float radius, Team::TeamType team) {
                    this->origin = origin;
                    this->radius = radius * radius;
                    this->team = team;
                    closest = NULL;
                }

                bool ReportFixture(b2Fixture *fixture) override {
                    b2Body *body = fixture->GetBody();

                    entt::entity e = (entt::entity)body->GetUserData().pointer;
                    entt::registry &regist = getRegistry();
                    if(
                        !regist.valid(e) ||
                        (regist.any_of<TeamComp>(e) && regist.get<TeamComp>(e).team == team) ||
                        (regist.any_of<HealthComp>(e) && !regist.get<HealthComp>(e).canHurt())
                    ) return true;

                    float range = (body->GetPosition() - origin).LengthSquared();
                    if(range > radius) return true;

                    if(closest == NULL || (closest->GetPosition() - origin).LengthSquared() > range) closest = body;
                    return true;
                }

                b2Body *get() {
                    return closest;
                }
            } report(pos, range, regist.get<TeamComp>(ref).team);

            b2Vec2 extent = b2Vec2(range, range);
            b2AABB bound;
            bound.lowerBound = pos - extent;
            bound.upperBound = pos + extent;

            world.QueryAABB(&report, bound);

            b2Body *target = report.get();
            if(target != NULL) {
                entt::entity bullet = (
                    type == SMALL ? App::instance->control->content->bulletSmall :
                    App::instance->control->content->bulletMed
                )->create(regist, world);

                regist.get<TeamComp>(bullet).team = regist.get<TeamComp>(ref).team;
                regist.get<TemporalComp>(bullet).range = range * 3.0f;

                b2Body *bbody = regist.get<RigidComp>(bullet).body;
                bbody->SetTransform(pos, angle(vec2(1.0f, 0.0f), normalize(vec2(bbody->GetPosition().x, bbody->GetPosition().y) - vec2(pos.x, pos.y))));

                b2Vec2 impulse = target->GetPosition() - pos;
                impulse.Normalize();
                impulse *= this->impulse;
                bbody->ApplyLinearImpulseToCenter(impulse, true);

                time = Time::time();
            }
        }
    }

    TemporalComp::TemporalComp(entt::entity e, TemporalFlag flags): Component(e) {
        this->flags = flags;
        range = 0.0f;
        time = 0.0f;
        init = false;
        initTime = 0.0f;
        initPos = b2Vec2_zero;
    }

    void TemporalComp::update() {
        entt::registry &regist = getRegistry();
        bool ranged = (flags & RANGE) == RANGE;
        bool timed = (flags & TIME) == TIME;

        b2Body *body = ranged ? regist.get<RigidComp>(ref).body : NULL;

        if(!init) {
            init = true;
            if(timed) initTime = Time::time();
            if(ranged) initPos = body->GetPosition();
        }

        if(timed && (Time::time() - initTime) >= time) {
            remove();
            return;
        }

        if(ranged && (body->GetPosition() - initPos).Length() >= range) {
            remove();
            return;
        }
    }
}
