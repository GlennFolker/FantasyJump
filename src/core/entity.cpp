#include <glm/gtx/vector_angle.hpp>

#include "entity.h"
#include "events.h"
#include "time.h"
#include "../app.h"
#include "../util/mathf.h"

namespace Fantasy {
    Component::Component(entt::entity e) { ref = e; }

    void Component::update() {}
    void Component::remove() { App::icontrol().scheduleRemoval(ref); }
    entt::entity Component::getRef() { return ref; }

    entt::entity Component::createFx(const std::string &effect, bool follow) {
        entt::registry &registry = App::iregistry();
        entt::entity fx = App::icontent().getByName<EffectType>(effect)->create();

        if(registry.any_of<RigidComp>(ref)) {
            RigidComp &other = registry.get<RigidComp>(fx);
            RigidComp &self = registry.get<RigidComp>(ref);

            other.body->SetTransform(self.body->GetPosition(), 0.0f);
            if(follow) other.body->SetLinearVelocity(self.body->GetLinearVelocity());
        }

        return fx;
    }

    int Component::createSfx(Mix_Chunk *sound) {
        entt::registry &registry = App::iregistry();
        int channel = Mix_PlayChannel(-1, sound, 0);
        if(channel < 0) return channel;

        if(registry.any_of<RigidComp>(ref)) {
            b2Vec2 pos = b2Vec2(App::irenderer().pos.x, App::irenderer().pos.y) - registry.get<RigidComp>(ref).body->GetPosition();
            float angle = glm::degrees(glm::orientedAngle(glm::vec2(0.0f, 1.0f), glm::normalize(glm::vec2(pos.x, pos.y))));
            angle = fmodf(angle, 360.0f);
            angle += 360.0f;
            angle = fmodf(angle, 360.0f);

            Mix_SetPosition(channel, angle, pos.Length() * 2.0f);
        }

        return channel;
    }

    RigidComp::RigidComp(entt::entity e, b2Body *body): Component(e) {
        this->body = body;
        this->body->GetUserData().pointer = (uintptr_t)ref;
        rotateSpeed = 0.0f;
        spawned = false;
        spawnSfx = deathSfx = NULL;
    }

    void RigidComp::update() {
        if(!spawned) {
            spawned = true;
            if(!spawnFx.empty()) createFx(spawnFx);
            if(spawnSfx) createSfx(spawnSfx);
        }

        if(!Mathf::near(rotateSpeed, 0.0f)) body->SetTransform(body->GetPosition(), body->GetAngle() + rotateSpeed);
    }

    void RigidComp::beginCollide(RigidComp &other) {
        entt::registry &registry = App::iregistry();
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
        entt::registry &regist = App::iregistry();
        if(!regist.any_of<TeamComp>(ref) || !regist.any_of<TeamComp>(other.ref)) {
            return true;
        } else {
            return regist.get<TeamComp>(ref).team != regist.get<TeamComp>(other.ref).team;
        }
    }

    void RigidComp::onDestroy(entt::registry &registry, entt::entity entity) {
        RigidComp &comp = registry.get<RigidComp>(entity);
        if(!App::icontrol().isResetting()) {
            if(!comp.deathFx.empty()) comp.createFx(comp.deathFx);
            if(comp.deathSfx) comp.createSfx(comp.deathSfx);
        }

        App::iworld().DestroyBody(comp.body);
    }

    DrawComp::DrawComp(entt::entity e, const std::string &drawer): DrawComp(e, drawer, 1.0f, 1.0f) {}
    DrawComp::DrawComp(entt::entity e, const std::string &drawer, float size): DrawComp(e, drawer, size, size) {}
    DrawComp::DrawComp(entt::entity e, const std::string &drawer, float width, float height): DrawComp(e, drawer, width, height, 0.0f) {}
    DrawComp::DrawComp(entt::entity e, const std::string &drawer, float width, float height, float z): Component(e) {
        this->drawer = drawer;
        this->width = width;
        this->height = height;
        this->z = z;
        region = std::nullopt;
    }

    void DrawComp::update() {
        entt::registry &registry = App::iregistry();
        b2Transform trns = registry.get<RigidComp>(ref).body->GetTransform();

        if(registry.any_of<HealthComp>(ref)) {
            float alpha = fmaxf(1.0f - (Time::time() - registry.get<HealthComp>(ref).hitTime) / 0.5f, 0.0f);
            App::ibatch().tint(Color(0.8f, 0.0f, 0.1f, alpha));
        }

        App::icontent().getByName<DrawType>(drawer)->drawer(ref);
        App::ibatch().tint(Color());
    }

    JumpComp::JumpComp(entt::entity e, float force, float timeout): Component(e) {
        this->force = force;
        this->timeout = timeout;
        holding = false;
        jumping = false;
        time = -1.0f;
        sound = NULL;
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

    bool JumpComp::isHolding() { return holding; }
    float JumpComp::getTime() { return time; }

    void JumpComp::update() {
        b2Body *body = App::iregistry().get<RigidComp>(ref).body;
        float now = Time::time();

        if(holding) {
            b2Vec2 vel = -body->GetLinearVelocity();
            vel *= (1.0f * fminf((now - time) / 0.08f, 1.0f)) * force / 2.0f;

            body->ApplyForceToCenter(vel, true);
            if(!Mathf::near(body->GetAngularVelocity(), 1.0f)) {
                body->ApplyAngularImpulse(body->GetAngularVelocity() * -(force * 0.001f), true);
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

            if(!effect.empty()) createFx(effect);
        }
    }

    HealthComp::HealthComp(entt::entity e, float health): HealthComp(e, health, 0.0f) {}
    HealthComp::HealthComp(entt::entity e, float health, float damage) : HealthComp(e, health, damage, 0.0f) {}
    HealthComp::HealthComp(entt::entity e, float health, float damage, float regeneration): Component(e) {
        this->health = maxHealth = health;
        this->damage = damage;
        this->regeneration = regeneration;
        hitTime = -1000.0f;
        selfDamage = false;
        showBar = true;
        dead = false;
    }

    void HealthComp::update() {
        if(canHurt() && !dead && health <= 0.0f) killed();
        if(!dead) health = fminf(health + regeneration, maxHealth);
    }

    void HealthComp::kill() {
        health = 0.0f;
    }

    void HealthComp::killed() {
        if(dead) return;
        dead = true;

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
            if(canHurt() && health <= 0.0f) killed();
        }
    }

    bool HealthComp::canHurt() {
        return health != -1.0f;
    }

    TeamComp::TeamComp(entt::entity e): TeamComp(e, Team::GENERIC) {}
    TeamComp::TeamComp(entt::entity e, float priority) : TeamComp(e, Team::GENERIC, priority) {}
    TeamComp::TeamComp(entt::entity e, Team::TeamType team): TeamComp(e, team, 1.0f) {}
    TeamComp::TeamComp(entt::entity e, Team::TeamType team, float priority): Component(e) {
        this->team = team;
        this->priority = priority;
    }

    ShooterComp::ShooterComp(entt::entity e, const std::string &bullet, float rate): ShooterComp(e, bullet, rate, 4.0f) {}
    ShooterComp::ShooterComp(entt::entity e, const std::string &bullet, float rate, float impulse): ShooterComp(e, bullet, rate, impulse, 20.0f) {}
    ShooterComp::ShooterComp(entt::entity e, const std::string &bullet, float rate, float impulse, float range): Component(e) {
        this->bullet = bullet;
        this->rate = rate;
        this->impulse = impulse;
        this->range = range;
        lastShoot = timer = Time::time();
        inaccuracy = 0.0f;
        shootSfx = NULL;
    }

    void ShooterComp::update() {
        entt::registry &regist = App::iregistry();
        b2World &world = App::iworld();
        b2Body *body = regist.get<RigidComp>(ref).body;

        float time = Time::time();
        if(time - lastShoot >= rate && time - timer >= 0.1f) {
            timer = time;
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
                    entt::registry &registry = App::iregistry();
                    if(
                        !registry.valid(e) ||
                        (!registry.any_of<TeamComp>(e) || registry.get<TeamComp>(e).team == team) ||
                        (!registry.any_of<HealthComp>(e) || !registry.get<HealthComp>(e).canHurt())
                    ) return true;

                    float range = (body->GetPosition() - origin).LengthSquared();
                    if(range > radius) return true;

                    if(closest == NULL) {
                        closest = body;
                        return true;
                    }

                    entt::entity b = (entt::entity)closest->GetUserData().pointer;
                    float brange = (closest->GetPosition() - origin).LengthSquared();
                    
                    if(priority(b) * (1.0f - brange / radius) < priority(e) * (1.0f - range / radius)) closest = body;
                    return true;
                }

                float priority(entt::entity e) {
                    entt::registry &registry = App::iregistry();
                    return registry.any_of<TeamComp>(e) ? registry.get<TeamComp>(e).priority : 1.0f;
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
                if(shootSfx) createSfx(shootSfx);
                if(!shootFx.empty()) createFx(shootFx, true);
                entt::entity bullet = App::icontent().getByName<EntityType>(this->bullet)->create();

                regist.get<TeamComp>(bullet).team = regist.get<TeamComp>(ref).team;
                regist.get<TemporalComp>(bullet).range = range * 1.4f;

                b2Body *bbody = regist.get<RigidComp>(bullet).body;
                bbody->SetTransform(pos, glm::orientedAngle(glm::vec2(1.0f, 0.0f), glm::normalize(glm::vec2(target->GetPosition().x - pos.x, target->GetPosition().y - pos.y))));

                b2Vec2 impulse = target->GetPosition() - pos;
                impulse.Normalize();
                impulse *= this->impulse;
                bbody->ApplyLinearImpulseToCenter(impulse, true);
                body->ApplyLinearImpulseToCenter(-impulse, true);

                lastShoot = time;
            }
        }
    }

    TemporalComp::TemporalComp(entt::entity e, TemporalFlag flags): Component(e) {
        this->flags = flags;
        range = 0.0f;
        time = 0.0f;
        initTime = Time::time();
        travelled = 0.0f;
    }

    void TemporalComp::update() {
        entt::registry &regist = App::iregistry();
        bool timed = (flags & TIME) == TIME;
        bool ranged = (flags & RANGE) == RANGE;

        if(timed && (Time::time() - initTime) >= time) {
            remove();
            return;
        }

        if(ranged) {
            travelled += regist.get<RigidComp>(ref).body->GetLinearVelocity().Length() / 60.0f;
            if(travelled >= range) remove();
        }
    }

    float TemporalComp::rangef() { return Mathf::clamp(travelled / range); }
    float TemporalComp::timef() { return Mathf::clamp((Time::time() - initTime) / time); }

    IdentifierComp::IdentifierComp(entt::entity e, const std::string &id): Component(e) {
        this->id = id;
    }
}
