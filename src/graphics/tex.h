#ifndef TEX_H
#define TEX_H

#include <SDL_image.h>
#include <gl/glew.h>

namespace Fantasy {
    class Tex {
        protected:
        unsigned int data;
        SDL_Surface *surface;

        public:
        Tex(const char *);
        Tex(SDL_Surface *);
        ~Tex();

        void load();
        virtual void bind() {}
        virtual void set(SDL_Surface *, bool bind = true) {}
        int active(int);
        void setWrap(int, int, int, bool bind = true);
        void setFilter(int, int, bool bind = true);
    };

    class Tex2D: public Tex {
        public:
        int width;
        int height;

        public:
        Tex2D(const char *filename);
        Tex2D(SDL_Surface *data);

        public:
        virtual void bind() override;
        virtual void set(SDL_Surface *, bool) override;
    };
}

#endif
