#include "team.h"

namespace Fantasy {
    const Color Team::colors[Team::ALL] = {
        Color::blue,
        Color::green,
        Color::gray
    };

    Color Team::color(TeamType team) {
        return colors[team];
    }
}
