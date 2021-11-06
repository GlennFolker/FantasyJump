#include <SDL.h>
#include <SDL_version.h>
#include <SDL_image.h>
#include <gl/glew.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <exception>
#include <string>

#include "app.h"
#include "core/renderer.h"

namespace Fantasy {
    App *App::instance = NULL;

    App::App(int argc, char *argv[], AppConfig config) {
        if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) throw std::exception(std::string("Couldn't initialize SDL: ").append(SDL_GetError()).c_str());

        SDL_Log("Initialized SDL v%d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

        SDL_Rect viewport = {
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            config.width, config.height
        };
        
        int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
        if(config.fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;

            SDL_DisplayMode mode;
            SDL_GetCurrentDisplayMode(0, &mode);

            viewport.w = mode.w;
            viewport.h = mode.h;
        }

        if(config.borderless) SDL_WINDOW_BORDERLESS;
        if(config.resizable) flags |= SDL_WINDOW_RESIZABLE;

        int imgFlags = IMG_INIT_PNG;
        if((IMG_Init(imgFlags) & ~imgFlags) != 0) throw std::exception(std::string("Couldn't initialize SDL_image: ").append(IMG_GetError()).c_str());
        
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        window = SDL_CreateWindow("Fantasy", viewport.x, viewport.y, viewport.w, viewport.h, flags);
        if(window == NULL) throw std::exception(std::string("Couldn't create SDL window: ").append(SDL_GetError()).c_str());
        
        context = SDL_GL_CreateContext(window);
        if(context == NULL) throw std::exception(std::string("Couldn't create OpenGL context: ").append(SDL_GetError()).c_str());

        SDL_Log("GL version: %s", glGetString(GL_VERSION));

        GLenum error = GL_NO_ERROR;
        glewExperimental = GL_TRUE;
        if((error = glewInit()) != GL_NO_ERROR) throw std::exception(std::string("Couldn't initialize GLEW").append((char *)glewGetErrorString(error)).c_str());
        
        if(SDL_GL_SetSwapInterval(1) != 0) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "VSync disabled.");

        exiting = false;
        instance = this;
        pos = dvec2(0.0, 0.0);
        proj = identity<dmat4>();
        flipProj = identity<fmat4>();

        input = new Input();
        listeners = new std::vector<AppListener *>();
        listeners->push_back(control = new GameController());
        listeners->push_back(renderer = new Renderer());
    }

    App::~App() {
        for(auto listener : *listeners) delete listener;
        delete listeners;
        delete input;

        SDL_DestroyWindow(window);
        SDL_GL_DeleteContext(context);
        IMG_Quit();
        SDL_Quit();
    }

    bool App::run() {
        SDL_Event e;
        while(!exiting) {
            double w = getWidth() / 48.0, h = getHeight() / 48.0;
            proj = ortho(
                pos.x - w, pos.x + w,
                pos.y - h, pos.y + h
            );
            flipProj = ortho(
                pos.x - w, pos.x + w,
                pos.y + h, pos.y - h
            );

            while(SDL_PollEvent(&e) != 0) {
                switch(e.type) {
                    case SDL_QUIT:
                        exiting = true;
                        break;
                    default:
                        input->read(e);
                }
            }
            
            try {
                for(auto it : *listeners) it->update();
            } catch(std::exception &e) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, e.what());
                return false;
            }

            SDL_GL_SwapWindow(window);
        }

        return true;
    }

    void App::exit() {
        exiting = true;
    }

    void App::getViewport(int *w, int *h) {
        SDL_GetWindowSize(window, w, h);
    }

    int App::getWidth() {
        int w;
        getViewport(&w, NULL);

        return w;
    }

    int App::getHeight() {
        int h;
        getViewport(NULL, &h);

        return h;
    }

    void App::getMouse(int *x, int *y) {
        SDL_GetMouseState(x, y);
    }

    int App::getMouseX() {
        int x;
        getMouse(&x, NULL);

        return x;
    }
    
    int App::getMouseY() {
        int y;
        getMouse(NULL, &y);

        return y;
    }

    float App::getAspect() {
        return (float)getWidth() / (float)getHeight();
    }

    void App::unproject(double x, double y, double *newX, double *newY) {
        double unusedX, unusedY, unusedZ;
        int viewport[4];
        viewport[0] = viewport[1] = 0;
        getViewport(&viewport[2], &viewport[3]);

        gluUnProject(
            x, y, 0.0,
            value_ptr(identity<dmat4>()), value_ptr(flipProj), viewport,
            newX == NULL ? &unusedX : newX, newY == NULL ? &unusedY : newY, &unusedZ
        );
    }
}
