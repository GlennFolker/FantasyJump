#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>
#include <vector>
#include <functional>

namespace Fantasy {
    struct InputContext {
        public:
        const void *data;
        bool performed;

        InputContext() {
            data = NULL;
            performed = false;
        }

        template<typename T>
        InputContext(const T &value) {
            data = &value;
            performed = false;
        }

        template<typename T>
        void set(const T &value) {
            data = &value;
        }

        template<typename T>
        const T &read() {
            return *(const T *)data;
        }
    };

    class Input {
        public:
        enum InputType {
            MOUSE,
            KEYBOARD
        };

        public:
        std::vector<std::function<void(InputContext &)>> *mouse;
        std::vector<std::function<void(InputContext &)>> *keyboard;

        public:
        Input();
        ~Input();

        void read(SDL_Event e);
        void attach(InputType type, const std::function<void(InputContext &)> &);
        void detach(InputType type, const std::function<void(InputContext &)> &);

        private:
        void remove(std::vector<std::function<void(InputContext &)>> *, const std::function<void(InputContext &)> &);
    };
}

#endif
