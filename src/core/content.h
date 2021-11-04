#ifndef CONTENT_H
#define CONTENT_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <entt/entity/registry.hpp>
#include <box2d/box2d.h>

namespace Fantasy {
    enum class CType {
        ENTITY,
        ALL
    };

    class Content {
        public:
        const char *name;

        public:
        Content(const char *);
    };

    class EntityType: public Content {
        public:
        std::function<void(entt::registry &, b2World &, entt::entity)> initializer;

        public:
        EntityType(const char *, std::function<void(entt::registry &, b2World &, entt::entity)> &&);
        entt::entity create(entt::registry &, b2World &);

        static CType ctype();
    };

    class Contents {
        private:
        std::vector<std::unordered_map<const char *, Content *> *> *contents;

        public:
        EntityType *jumper, *spike;

        public:
        Contents();
        ~Contents();

        template<
            typename T, typename... Args,
            std::enable_if<std::is_base_of<Content, T>::value>::type *_T = nullptr
        > T *create(Args&&... args) {
            T *content = new T(args...);

            std::unordered_map<const char *, Content *> *map = getBy(T::ctype());
            if(map->contains(content->name)) {
                throw std::exception(std::string("'").append(typeid(T).name()).append("' with name '").append(content->name).append("' already exists.").c_str());
            } else {
                map->emplace(content->name, content);
            }

            return content;
        }

        std::unordered_map<const char *, Content *> *getBy(CType);
    };
}

#endif
