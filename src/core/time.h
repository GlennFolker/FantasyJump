#ifndef TIME_H
#define TIME_H

#include <chrono>

namespace Fantasy {
    class Time {
        private:
        static const std::chrono::nanoseconds init;

        public:
        static float time();
        static std::chrono::nanoseconds now();
    };
}

#endif
