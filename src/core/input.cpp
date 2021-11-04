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
                for(std::function<void(InputContext &)> &func : *mouse) {
                    func(ctx);
                }
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                ctx.performed = e.type == SDL_KEYDOWN;
                ctx.set(e.key.keysym);
                for(std::function<void(InputContext &)> &func : *keyboard) {
                    func(ctx);
                }
                break;
        }
    }

    void Input::attach(SDL_EventType type, std::function<void(InputContext &)> &&func) {
        switch(type) {
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                mouse->push_back(func);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                keyboard->push_back(func);
                break;
        }
    }

    void Input::detach(SDL_EventType type, std::function<void(InputContext &)> &&func) {
        switch(type) {
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                for(auto it = mouse->begin(); it != mouse->end(); it++) {
                    if((*it).target_type() == func.target_type()) {
                        mouse->erase(it);
                        break;
                    }
                }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                for(auto it = keyboard->begin(); it != keyboard->end(); it++) {
                    if((*it).target_type() == func.target_type()) {
                        keyboard->erase(it);
                        break;
                    }
                }
                break;
        }
    }
}
