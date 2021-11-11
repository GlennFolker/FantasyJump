#ifndef TEAM_H
#define TEAM_H

#include "../graphics/color.h"

namespace Fantasy {
    class Team {
        private:
        static const Color colors[];

        public:
        enum TeamType {
            BLUE,
            GREEN,
            GENERIC,
            ALL
        };

        public:
        static Color color(TeamType);
    };
}

#endif
