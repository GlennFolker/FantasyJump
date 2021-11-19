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
        int width, height;
        bool hasColor, hasDepth, hasStencil;

        public:
        FrameBuffer();
        FrameBuffer(int, int);
        FrameBuffer(int, int, bool);
        FrameBuffer(int, int, bool, bool);
        FrameBuffer(int, int, bool, bool, bool);
        ~FrameBuffer();

        void resize(int width, int height);
        void begin();
        void end();
    };
}

#endif
