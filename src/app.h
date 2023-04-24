#ifndef APP_H
#define APP_H

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <SDL.h>
#include <SDL_opengl.h>
#include <vector>

#include "app_listener.h"
#include "core/renderer.h"
#include "core/input.h"
#include "core/game_controller.h"

namespace Fantasy {
    struct AppConfig {
        int width = 800;
        int height = 600;
        bool visible;
        bool fullscreen;
        bool resizable;
        bool borderless;
    };

    class App {
        public:
        static App *instance;

        std::vector<AppListener *> *listeners;
        SDL_Window *window;
        SDL_GLContext context;

        Input *input;
        GameController *control;
        Renderer *renderer;

        private:
        bool exiting;
        bool fullscreen;
        int lastWidth, lastHeight;

        public:
        App(int, char *[], AppConfig);
        ~App();

        bool run();
        void exit();

        bool isFullscreen();
        void setFullscreen(bool);
        void getViewport(int *, int *);
        int getWidth();
        int getHeight();
        void getMouse(int *, int *);
        int getMouseX();
        int getMouseY();
        float getAspect();

        static inline GameController &icontrol() { return *instance->control; }
        static inline Contents &icontent() { return *instance->control->content; }
        static inline entt::registry &iregistry() { return *instance->control->regist; }
        static inline b2World &iworld() { return *instance->control->world; }
        static inline Renderer &irenderer() { return *instance->renderer; }
        static inline TexAtlas &iatlas() { return *instance->renderer->atlas; }
        static inline SpriteBatch &ibatch() { return *instance->renderer->batch; }
    };
}

#endif
