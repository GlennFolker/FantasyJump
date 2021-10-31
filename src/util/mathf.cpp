#include "mathf.h"

namespace Fantasy {
    bool Mathf::near(float a, float b) {
        return near(a, b, 0.00001f);
    }

    bool Mathf::near(float a, float b, float threshold) {
        return fabs(a - b) < threshold;
    }
}
