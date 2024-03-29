#ifndef TEX_H
#define TEX_H

#include <SDL_image.h>
#include <GL/glew.h>

namespace Fantasy {
    class Tex {
        public:
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
        virtual void setWrap(int, int, int, bool bind = true) {}
        virtual void setFilter(int, int, bool bind = true) {}
    };

    class Tex2D: public Tex {
        public:
        int width;
        int height;

        public:
        Tex2D(const char *);
        Tex2D(SDL_Surface *);
        Tex2D(int, int, SDL_Surface *);

        public:
        virtual void bind() override;
        virtual void set(SDL_Surface *, bool bind = true) override;
        void setWrap(int, int, int, bool bind = true) override;
        void setFilter(int, int, bool bind = true) override;
    };
}

#endif
