#include <gl/glew.h>
#include <SDL.h>

#include "app.h"

using namespace Fantasy;

int main(int argc, char *argv[]) {
    App *app = new App(&argc, &argv);
    if(app != NULL && app->run()) {
        app->~App();

        SDL_Log("Ended successful launch, program exited with code 0.\n");
        return 0;
    } else {
        if(app != NULL) app->~App();

        SDL_Log("Program exited with code 1.\n");
        return 1;
    }
}
