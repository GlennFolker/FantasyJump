#include "color.h"

namespace Fantasy {
    Color::Color(): Color(0.0f, 0.0f, 0.0f, 0.0f) {}

    Color::Color(float r, float g, float b): Color(r, g, b, 1.0f) {}

    Color::Color(float r, float g, float b, float a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    unsigned int Color::abgr() {
        int a = (int)(this->a * 255.0f) << 24;
        int b = (int)(this->b * 255.0f) << 16;
        int g = (int)(this->g * 255.0f) << 8;
        int r = (int)(this->r * 255.0f);
        return a | b | g | r;
    }

    Color Color::fromAbgr(unsigned int abgr) {
        a = ((abgr & 0xff000000) >> 24) / 255.0f;
        b = ((abgr & 0x00ff0000) >> 16) / 255.0f;
        g = ((abgr & 0x0000ff00) >> 8) / 255.0f;
        r = (abgr & 0x000000ff) / 255.0f;
        return *this;
    }

    float Color::fabgr() {
        int bits = abgr();
        return *(float *)&bits;
    }

    Color Color::fromFagbr(float abgr) {
        return fromAbgr(*(unsigned int *)&abgr);
    }
}
