#ifndef MATHF_H
#define MATHF_H

#include <cmath>
#include <numbers>

namespace Fantasy {
    class Mathf {
        public:
        static inline bool near(float, float);
        static inline bool near(float, float, float);
    };
}

#endif
