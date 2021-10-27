#ifndef APP_H
#define APP_H

#include <gl/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <vector>

#include "app_listener.h"
#include "core/renderer.h"

namespace Fantasy {
    class App {
        public:
        static App *instance;

        std::vector<AppListener *> *listeners;
        SDL_Window *window = NULL;
        SDL_GLContext context;

        Renderer *renderer;

        private:
        bool exiting;

        public:
        App(int *, char **[]);
        ~App();

        bool run();
        void exit();

        int getWidth();
        int getHeight();
    };
}

#endif
