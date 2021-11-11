#ifndef COLOR_H
#define COLOR_H

namespace Fantasy {
    struct Color {
        public:
        static const Color white;
        static const Color gray;
        static const Color red;
        static const Color green;
        static const Color blue;

        float r;
        float g;
        float b;
        float a;

        public:
        Color();
        Color(float, float, float);
        Color(float, float, float, float);

        unsigned int abgr();
        Color &fromAbgr(unsigned int);
        float fabgr();
        Color &fromFagbr(float);
    };
}

#endif
