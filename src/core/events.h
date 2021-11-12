#ifndef EVENTS_H
#define EVENTS_H

#include <entt/entity/registry.hpp>
#include <typeindex>
#include <functional>
#include <unordered_map>
#include <vector>

namespace Fantasy {
    class Event {};

    class Events {
        private:
        static std::unordered_map<std::type_index, std::vector<std::function<void(Event &)>>> *hooks;

        public:
        static void fire(const std::type_index &, Event &);
        static void fire(const std::type_index &, Event &&);
        static void on(const std::type_index &, std::function<void(Event &)> &);
        static void on(const std::type_index &, std::function<void(Event &)> &&);

        template<typename T, typename std::enable_if<std::is_base_of<Event, T>::value>::type *_T = nullptr>
        static void fire(Event &e) {
            fire(typeid(T), e);
        }

        template<typename T, typename std::enable_if<std::is_base_of<Event, T>::value>::type *_T = nullptr>
        static void fire(Event &&e) {
            fire(typeid(T), e);
        }

        template<typename T, typename std::enable_if<std::is_base_of<Event, T>::value>::type *_T = nullptr>
        static void on(std::function<void(Event &)> &func) {
            on(typeid(T), func);
        }

        template<typename T, typename std::enable_if<std::is_base_of<Event, T>::value>::type *_T = nullptr>
        static void on(std::function<void(Event &)> &&func) {
            on(typeid(T), func);
        }
    };

    class EntDeathEvent: public Event {
        public:
        entt::entity entity;

        public:
        EntDeathEvent();
        EntDeathEvent(entt::entity);
    };
}

#endif
