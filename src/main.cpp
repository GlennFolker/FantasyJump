#define GLEW_STATIC

#include <SDL.h>
#include <exception>

#include "app.h"

using namespace Fantasy;

int main(int argc, char *argv[]) {
    AppConfig config = AppConfig();
    config.width = 800;
    config.height = 600;

    App *app;
    try {
        app = new App(argc, argv, config);
    } catch(std::exception &e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, e.what());
        app = NULL;
    }

    if(app != NULL && app->run()) {
        app->~App();

        SDL_Log("Ended successful launch, program exited with code 0.");
        return 0;
    } else {
        if(app != NULL) app->~App();

        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Program exited with code 1.");
        return 1;
    }
}
