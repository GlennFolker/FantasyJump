#ifndef MATHF_H
#define MATHF_H

#include <cmath>
#include <numbers>

namespace Fantasy {
    class Mathf {
        public:
        static bool near(float, float);
        static bool near(float, float, float);
    };
}

#endif
