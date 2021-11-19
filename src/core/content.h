#ifndef CONTENT_H
#define CONTENT_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <stdexcept>
#include <string>
#include <entt/entity/registry.hpp>
#include <box2d/box2d.h>

#include "../graphics/tex_atlas.h"

namespace Fantasy {
    enum class CType {
        ENTITY,
        DRAW,
        EFFECT,
        ALL
    };

    class Content {
        public:
        std::string name;

        public:
        Content(const Content &) = delete;
        Content(Content &&) = delete;
        Content(const std::string &);
    };

    class EntityType: public Content {
        public:
        std::function<void(entt::entity)> initializer;

        public:
        EntityType(const std::string &, const std::function<void(entt::entity)> &);
        entt::entity create();

        static CType ctype();
    };

    class DrawType: public Content {
        public:
        std::function<void(entt::entity)> drawer;

        public:
        DrawType(const std::string &, const std::function<void(entt::entity)> &);

        static CType ctype();
    };

    class EffectType: public EntityType {
        public:
        float clipSize, lifetime, z;
        std::string drawer;

        public:
        EffectType(const std::string &, const std::string &);
        EffectType(const std::string &, DrawType *);
        EffectType(const std::string &, const std::function<void(entt::entity)> &, const std::string &);
        EffectType(const std::string &, const std::function<void(entt::entity)> &, DrawType *);

        static CType ctype();
    };

    class Contents {
        private:
        std::vector<std::unordered_map<std::string, Content *> *> *contents;

        public:
        DrawType
            *genericRegion;

        EntityType
            *jumper, *spike, *leak,
            *bulletSmall, *bulletMed;

        EffectType
            *smokeSmall, *smokeBig,
            *destructSmall, *destructMed, *destructBig;

        public:
        Contents();
        ~Contents();

        template<
            typename T, typename... Args,
            typename std::enable_if<std::is_base_of<Content, T>::value>::type *_T = nullptr
        > T *create(const Args&... args) {
            T *content = new T(args...);
            if(content->name.empty()) throw std::runtime_error("Content name can't be empty.");

            std::unordered_map<std::string, Content *> *map = getBy(T::ctype());
            if(map->count(content->name)) {
                throw std::runtime_error(std::string("'").append(typeid(T).name()).append("' with name '").append(content->name).append("' already exists.").c_str());
            } else {
                map->emplace(content->name, content);
            }

            return content;
        }

        template<typename T, typename std::enable_if<std::is_base_of<Content, T>::value>::type *_T = nullptr>
        T *getByName(const std::string &name) {
            if(name.empty()) throw std::runtime_error("Content name can't be empty.");

            std::unordered_map<std::string, Content *> *map = getBy(T::ctype());
            if(!map->count(name)) {
                throw std::runtime_error(std::string("'").append(typeid(T).name()).append("' with name '").append(name).append("' doesn't exist.").c_str());
            } else {
                return (T *)map->at(name);
            }
        }

        std::unordered_map<std::string, Content *> *getBy(CType);
    };
}

#endif
