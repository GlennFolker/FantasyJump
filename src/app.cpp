#include <SDL.h>
#include <SDL_version.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <stdexcept>
#include <string>

#include "app.h"
#include "core/renderer.h"
#include "core/events.h"

namespace Fantasy {
    App *App::instance = nullptr;

    App::App(int argc, char *argv[], AppConfig config) {
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) throw std::runtime_error(std::string("Couldn't initialize SDL: ").append(SDL_GetError()).c_str());
        SDL_Log("Initialized SDL v%d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

        int flags = SDL_WINDOW_OPENGL;
        if(config.visible) flags |= SDL_WINDOW_SHOWN;
        if(config.borderless) flags |= SDL_WINDOW_BORDERLESS;
        if(config.resizable) flags |= SDL_WINDOW_RESIZABLE;

        if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) throw std::runtime_error(std::string("Couldn't initialize SDL_image: ").append(IMG_GetError()).c_str());
        SDL_Log("Initialized SDL_image v%d.%d.%d", SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);

        if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) throw std::runtime_error(std::string("Couldn't initialize SDL_mixer: ").append(Mix_GetError()).c_str());
        if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 2048) != 0) throw std::runtime_error(std::string("Couldn't initialize SDL_mixer: ").append(Mix_GetError()).c_str());
        SDL_Log("Initialized SDL_mixer v%d.%d.%d", SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL);
        SDL_Log("Using %d audio channels.", Mix_AllocateChannels(32));

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        lastWidth = config.width;
        lastHeight = config.height;
        window = SDL_CreateWindow("Fantasy Jump", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.width, config.height, flags);
        if(window == nullptr) throw std::runtime_error(std::string("Couldn't create SDL window: ").append(SDL_GetError()).c_str());

        context = SDL_GL_CreateContext(window);
        if(context == nullptr) throw std::runtime_error(std::string("Couldn't create OpenGL context: ").append(SDL_GetError()).c_str());

        SDL_Log("OpenGL version: %s", glGetString(GL_VERSION));

        GLenum error = GL_NO_ERROR;
        glewExperimental = GL_TRUE;
        if((error = glewInit()) != GL_NO_ERROR) throw std::runtime_error(std::string("Couldn't initialize GLEW: ").append((const char *)glewGetErrorString(error)).c_str());

        if(SDL_GL_SetSwapInterval(1) != 0) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "VSync disabled.");

        exiting = false;
        instance = this;

        input = new Input();
        listeners = new std::vector<AppListener *>();
        listeners->push_back(control = new GameController());
        listeners->push_back(renderer = new Renderer());

        setFullscreen(config.fullscreen);
        Events::fire<AppLoadEvent>(AppLoadEvent());
    }

    App::~App() {
        for(auto listener : *listeners) delete listener;
        delete listeners;
        delete input;

        SDL_DestroyWindow(window);
        SDL_GL_DeleteContext(context);
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    bool App::run() {
        SDL_Event e;
        while(!exiting) {
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
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", e.what());
                return false;
            }

            SDL_GL_SwapWindow(window);
        }

        return true;
    }

    bool App::isFullscreen() { return fullscreen; }
    void App::setFullscreen(bool fullscreen) {
        if(this->fullscreen == fullscreen) return;
        this->fullscreen = fullscreen;

        if(fullscreen) {
            getViewport(&lastWidth, &lastHeight);
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else {
            SDL_SetWindowFullscreen(window, false);
            SDL_SetWindowSize(window, lastWidth, lastHeight);
            SDL_SetWindowPosition(window, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED);
        }
    }

    void App::exit() { exiting = true; }
    void App::getViewport(int *w, int *h) { SDL_GetWindowSize(window, w, h); }

    int App::getWidth() {
        int w;
        getViewport(&w, nullptr);

        return w;
    }

    int App::getHeight() {
        int h;
        getViewport(nullptr, &h);

        return h;
    }

    void App::getMouse(int *x, int *y) { SDL_GetMouseState(x, y); }
    int App::getMouseX() {
        int x;
        getMouse(&x, nullptr);

        return x;
    }

    int App::getMouseY() {
        int y;
        getMouse(nullptr, &y);

        return y;
    }

    float App::getAspect() { return (float)getWidth() / (float)getHeight(); }
}
