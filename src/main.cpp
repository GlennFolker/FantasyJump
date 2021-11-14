#include <SDL.h>
#include <SDL_main.h>
#include <stdexcept>
#include <glm/gtx/vector_angle.hpp>

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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", e.what());
        app = NULL;
    }

    if(app != NULL && app->run()) {
        delete app;

        SDL_Log("Ended successful launch, program exited with code 0.");
        return 0;
    } else {
        delete app;

        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Program exited with code 1.");
        return 1;
    }
}
