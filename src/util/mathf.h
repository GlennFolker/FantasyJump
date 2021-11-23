#ifndef MATHF_H
#define MATHF_H

#include <math.h>
#include <stdlib.h>
#include <functional>
#include <glm/gtx/transform.hpp>

namespace Fantasy {
    class Mathf {
        public:
        static inline bool near(float a, float b) { return near(a, b, 0.00001f); }
        static inline bool near(float a, float b, float epsilon) { return fabs(a - b) < epsilon; }

        static inline float random() { return random(0.0f, 1.0f); }
        static inline float random(float mag) { return random(0.0f, mag); }
        static inline float random(float from, float to) { return from + ((float)rand() / (float)RAND_MAX) * (to - from); }

        static inline float srandom(unsigned int seed) { return srandom(seed, 0.0f, 1.0f); }
        static inline float srandom(unsigned int seed, float mag) { return srandom(seed, 0.0f, mag); }
        static inline float srandom(unsigned int seed, float from, float to) { srand(seed); return random(from, to); }

        static inline float lerp(float from, float to, float progress) { return from + (to - from) * progress; }

        static inline void randVecs(unsigned int seed, int amount, float maxLength, float progress, const std::function<void(float, float)> &func) {
            randVecs(seed, amount, 0.0f, maxLength, progress, 0.0f, glm::two_pi<float>(), 0.0f, [](float len) { return 1.0f; }, func);
        }

        static inline void randVecs(unsigned int seed, int amount, float maxLength, float progress, float offsetAngle, const std::function<void(float, float)> &func) {
            randVecs(seed, amount, 0.0f, maxLength, progress, 0.0f, glm::two_pi<float>(), offsetAngle, [](float len) { return 1.0f; }, func);
        }

        static inline void randVecs(unsigned int seed, int amount, float maxLength, float progress, float offsetAngle, const std::function<float(float)> &angleProg, const std::function<void(float, float)> &func) {
            randVecs(seed, amount, 0.0f, maxLength, progress, 0.0f, glm::two_pi<float>(), offsetAngle, angleProg, func);
        }

        static inline void randVecs(unsigned int seed, int amount, float minLength, float maxLength, float progress, float offsetAngle, const std::function<float(float)> &angleProg, const std::function<void(float, float)> &func) {
            randVecs(seed, amount, minLength, maxLength, progress, 0.0f, glm::two_pi<float>(), offsetAngle, angleProg, func);
        }

        static inline void randVecs(unsigned int seed, int amount, float minLength, float maxLength, float progress, float coneFrom, float coneTo, float offsetAngle, const std::function<float(float)> &angleProg, const std::function<void(float, float)> &func) {
            srand(seed);
            for(int i = 0; i < amount; i++) {
                float angle = random(coneFrom, coneTo);
                float len = random(minLength, maxLength) * progress;
                
                if(offsetAngle != 0.0f) angle += offsetAngle * angleProg(len);
                float x = glm::cos(angle) * len;
                float y = glm::sin(angle) * len;
                func(x, y);
            }
        }

        static inline float clamp(float value) { return clamp(value, 0.0f, 1.0f); }
        static inline float clamp(float value, float min, float max) {
            if(min > value) return min;
            if(max < value) return max;
            return value;
        }
    };
}

#endif
