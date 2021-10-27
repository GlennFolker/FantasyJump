#include <SDL.h>
#include <gl/glew.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>

#include "app.h"

namespace Fantasy {
    App::App(int *argc, char **argv[]) {
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize SDL: %s\n", SDL_GetError());
            this->~App();
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        SDL_Rect viewport = {SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600};
        SDL_DisplayMode mode;
        if(SDL_GetCurrentDisplayMode(0, &mode) == 0) {
            viewport.w = mode.w;
            viewport.h = mode.h;
        }

        window = SDL_CreateWindow("Fantasy", viewport.x, viewport.y, viewport.w, viewport.h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if(window == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't create SDL window: %s\n", SDL_GetError());
            this->~App();
        }
        
        context = SDL_GL_CreateContext(window);
        if(context == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't create GL context: %s\n", SDL_GetError());
            this->~App();
        }

        GLenum error = GL_NO_ERROR;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if((error = glGetError()) != GL_NO_ERROR) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize GL: %s\n", gluErrorString(error));
            this->~App();
        }

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        if((error = glGetError()) != GL_NO_ERROR) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize GL: %s\n", gluErrorString(error));
            this->~App();
        }
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        if((error = glGetError()) != GL_NO_ERROR) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize GL: %s\n", gluErrorString(error));
            this->~App();
        }

        glewExperimental = GL_TRUE;
        if((error = glewInit()) != GL_NO_ERROR) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize GLEW: %s\n", glewGetErrorString(error));
            this->~App();
        }
        
        if(SDL_GL_SetSwapInterval(1) != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "VSync disabled due to: %s\n", SDL_GetError());
        }

        exiting = false;
    }

    App::~App() {
        if(window != NULL) SDL_DestroyWindow(window);
        if(context != NULL) SDL_GL_DeleteContext(context);
        SDL_Quit();
    }

    void App::run() {
        SDL_Event e;
        while(!exiting) {
            while(SDL_PollEvent(&e) != 0) {
                if(e.type == SDL_QUIT) {
                    exiting = true;
                }
            }
        }
    }

    void App::exit() {
        exiting = true;
    }

    App *App::instance = NULL;
}
