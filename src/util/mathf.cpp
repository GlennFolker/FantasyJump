#include "mathf.h"

namespace Fantasy {
    bool Mathf::near(float a, float b) {
        return near(a, b, 0.00001f);
    }

    bool Mathf::near(float a, float b, float threshold) {
        return fabs(a - b) < threshold;
    }

    float Mathf::random() {
        return random(0.0f, 1.0f);
    }

    float Mathf::random(float mag) {
        return random(0.0f, mag);
    }

    float Mathf::random(float from, float to) {
        return from + ((float)rand() / (float)RAND_MAX) * (to - from);
    }
}
