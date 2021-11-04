#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>
#include <vector>
#include <functional>

namespace Fantasy {
    struct InputContext {
        public:
        void *data;
        bool performed;

        InputContext() {
            data = NULL;
            performed = false;
        }

        template<typename T>
        InputContext(T &&value) {
            data = &value;
            performed = false;
        }

        template<typename T>
        void set(T &&value) {
            data = &value;
        }

        template<typename T>
        T &read() {
            return *(T *)data;
        }
    };

    class Input {
        public:
        std::vector<std::function<void(InputContext &)>> *mouse;
        std::vector<std::function<void(InputContext &)>> *keyboard;

        public:
        Input();
        ~Input();

        void read(SDL_Event e);
        void attach(SDL_EventType type, std::function<void(InputContext &)> &&);
        void detach(SDL_EventType type, std::function<void(InputContext &)> &&);
    };
}

#endif
