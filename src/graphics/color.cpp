#include "color.h"
#include "../util/mathf.h"

namespace Fantasy {
    const Color Color::white = Color(1.0f, 1.0f, 1.0f, 1.0f);
    const Color Color::gray = Color(0.5f, 0.5f, 0.5f, 1.0f);
    const Color Color::red = Color(1.0f, 0.0f, 0.2f, 1.0f);
    const Color Color::lred = Color(1.0f, 0.3f, 0.2f, 1.0f);
    const Color Color::orange = Color(1.0f, 0.3f, 0.0f, 1.0f);
    const Color Color::lorange = Color(1.0f, 0.3f, 0.2f, 1.0f);
    const Color Color::yellow = Color(1.0f, 0.6f, 0.0f, 1.0f);
    const Color Color::lyellow = Color(1.0f, 0.6f, 0.3f, 1.0f);
    const Color Color::green = Color(0.1f, 1.0f, 0.2f, 1.0f);
    const Color Color::blue = Color(0.1f, 0.3f, 1.0f, 1.0f);
    const Color Color::purple = Color(0.6f, 0.0f, 1.0f, 1.0f);
    const Color Color::lpurple = Color(0.7f, 0.5f, 1.0f, 1.0f);

    Color::Color(): Color(0.0f, 0.0f, 0.0f, 0.0f) {}
    Color::Color(float r, float g, float b): Color(r, g, b, 1.0f) {}
    Color::Color(float r, float g, float b, float a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    unsigned int Color::abgr() const {
        int a = (int)(this->a * 255.0f) << 24;
        int b = (int)(this->b * 255.0f) << 16;
        int g = (int)(this->g * 255.0f) << 8;
        int r = (int)(this->r * 255.0f);
        return a | b | g | r;
    }

    Color &Color::fromAbgr(unsigned int abgr) {
        a = ((abgr & 0xff000000) >> 24) / 255.0f;
        b = ((abgr & 0x00ff0000) >> 16) / 255.0f;
        g = ((abgr & 0x0000ff00) >> 8) / 255.0f;
        r = (abgr & 0x000000ff) / 255.0f;
        return *this;
    }

    float Color::fabgr() const {
        int bits = abgr();
        return *(float *)&bits;
    }

    Color &Color::fromFagbr(float abgr) {
        return fromAbgr(*(unsigned int *)&abgr);
    }

    Color &Color::lerp(const Color &to, float progress) {
        r = Mathf::lerp(r, to.r, progress);
        g = Mathf::lerp(g, to.g, progress);
        b = Mathf::lerp(b, to.b, progress);
        a = Mathf::lerp(a, to.a, progress);
        return *this;
    }
}
