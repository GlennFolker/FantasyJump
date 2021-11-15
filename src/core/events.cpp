#include "events.h"

namespace Fantasy {
    std::unordered_map<std::type_index, std::vector<std::function<void(Event &)>>> *Events::hooks = new std::unordered_map<std::type_index, std::vector<std::function<void(Event &)>>>();
    
    void Events::fire(const std::type_index &type, Event &e) {
        if(!hooks->count(type)) return;

        std::vector<std::function<void(Event &)>> &listeners = hooks->at(type);
        for(std::function<void(Event &)> &func : listeners) func(e);
    }

    void Events::fire(const std::type_index &type, Event &&e) {
        if(!hooks->count(type)) return;

        std::vector<std::function<void(Event &)>> &listeners = hooks->at(type);
        for(std::function<void(Event &)> &func : listeners) func(e);
    }

    void Events::on(const std::type_index &type, std::function<void(Event &)> &func) {
        if(!hooks->count(type)) hooks->emplace(type, std::vector<std::function<void(Event &)>>());
        hooks->at(type).push_back(func);
    }

    void Events::on(const std::type_index &type, std::function<void(Event &)> &&func) {
        if(!hooks->count(type)) hooks->emplace(type, std::vector<std::function<void(Event &)>>());
        hooks->at(type).push_back(func);
    }

    EntDeathEvent::EntDeathEvent(): EntDeathEvent(entt::entity()) {}
    EntDeathEvent::EntDeathEvent(entt::entity entity) {
        this->entity = entity;
    }
}
