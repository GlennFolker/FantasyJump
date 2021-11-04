#ifndef APP_H
#define APP_H

#include <gl/glew.h>
#include <glm/mat4x4.hpp>
#include <SDL.h>
#include <SDL_opengl.h>
#include <vector>

#include "app_listener.h"
#include "core/assets.h"
#include "core/renderer.h"
#include "core/input.h"
#include "core/game_controller.h"

namespace Fantasy {
    struct AppConfig {
        int width;
        int height;
        bool fullscreen;
        bool resizable;
        bool borderless;
    };

    class App {
        public:
        static App *instance;

        std::vector<AppListener *> *listeners;
        SDL_Window *window = NULL;
        SDL_GLContext context;

        dmat4 proj;
        dmat4 flipProj;
        dvec2 pos;
        AssetManager *assets;
        Input *input;
        GameController *control;
        Renderer *renderer;

        private:
        bool exiting;

        public:
        App(int, char *[], AppConfig);
        ~App();

        bool run();
        void exit();

        void getViewport(int *, int *);
        int getWidth();
        int getHeight();
        void getMouse(int *, int *);
        int getMouseX();
        int getMouseY();
        float getAspect();
        void unproject(double, double, double *, double *);
    };
}

#endif
