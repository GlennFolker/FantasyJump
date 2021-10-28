#include <SDL.h>

namespace Fantasy {
    bool near(float a, float b, float threshold) {
        return SDL_fabs(a - b) < threshold;
    }

    bool near(float a, float b) {
        return near(a, b, 0.00001f);
    }
}
