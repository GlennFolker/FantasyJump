#include "time.h"

namespace Fantasy {
    const std::chrono::nanoseconds Time::init = Time::now();

    float Time::time() {
        return (now() - init).count() / 1000000000.0f;
    }

    std::chrono::nanoseconds Time::now() {
        return std::chrono::high_resolution_clock::now() - std::chrono::high_resolution_clock::time_point(std::chrono::seconds(0));
    }
}
