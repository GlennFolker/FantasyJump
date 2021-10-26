#define GL_GLEXT_PROTOTYPES

#include <SDL.h>

#include "app.h"

using namespace Fantasy;

int main(int argc, char *argv[]) {
    App *app = new App(&argc, &argv);
    if(app != NULL) {
        app->run();
        app->~App();

        SDL_Log("Ended successful launch.\n");
        return 0;
    } else {
        SDL_Log("Couldn't launch.\n");
        return 1;
    }
}
