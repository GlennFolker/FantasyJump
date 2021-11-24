#include "input.h"

namespace Fantasy {
    Input::Input() {
        mouse = new std::vector<std::function<void(InputContext &)>>();
        keyboard = new std::vector<std::function<void(InputContext &)>>();
    }

    Input::~Input() {
        delete mouse;
        delete keyboard;
    }

    void Input::read(SDL_Event e) {
        InputContext ctx;
        switch(e.type) {
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                ctx.performed = e.type == SDL_MOUSEBUTTONDOWN;
                ctx.set(e.button);
                for(std::function<void(InputContext &)> &func : *mouse) func(ctx);
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                ctx.performed = e.type == SDL_KEYDOWN;
                ctx.set(e.key);
                for(std::function<void(InputContext &)> &func : *keyboard) func(ctx);
                break;
        }
    }

    void Input::attach(InputType type, const std::function<void(InputContext &)> &func) {
        switch(type) {
            case MOUSE:
                mouse->push_back(func);
                break;
            case KEYBOARD:
                keyboard->push_back(func);
                break;
        }
    }

    void Input::detach(InputType type, const std::function<void(InputContext &)> &func) {
        switch(type) {
            case MOUSE:
                remove(mouse, func);
                break;
            case KEYBOARD:
                remove(keyboard, func);
                break;
        }
    }

    void Input::remove(std::vector<std::function<void(InputContext &)>> *array, const std::function<void(InputContext &)> &func) {
        for(auto it = array->begin(); it != array->end(); it++) {
            if((*it).target_type() == func.target_type()) {
                array->erase(it);
                break;
            }
        }
    }
}
