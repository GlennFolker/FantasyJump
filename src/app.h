#ifndef APP_H
#define APP_H

#include <SDL.h>
#include <SDL_opengl.h>

namespace Fantasy {
    class App {
        public:
        static App *instance;

        public:
        SDL_Window *window = NULL;
        SDL_GLContext context;

        private:
        bool exiting;

        public:
        App(int *, char **[]);

        ~App();

        void run();

        void exit();
    };
}

#endif
