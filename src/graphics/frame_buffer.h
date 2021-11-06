#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "tex.h"

namespace Fantasy {
    class FrameBuffer {
        private:
        static FrameBuffer *last;
        FrameBuffer *before;
        bool capturing;

        public:
        unsigned int data;
        Tex2D *texture;
        unsigned int render;

        public:
        FrameBuffer();
        FrameBuffer(int, int);
        FrameBuffer(int, int, bool);
        FrameBuffer(int, int, bool, bool);
        FrameBuffer(int, int, bool, bool, bool);
        ~FrameBuffer();

        void begin();
        void end();
    };
}

#endif
