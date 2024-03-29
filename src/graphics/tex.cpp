#include <SDL_image.h>
#include <cmath>
#include <string>
#include <exception>

#include "tex.h"
#include "../app.h"

namespace Fantasy {
    Tex::Tex(const char *filename): Tex(IMG_Load(filename)) {}
    Tex::Tex(SDL_Surface *surface) {
        this->surface = surface;
        data = 0;
    }

    Tex::~Tex() {
        glDeleteTextures(1, &data);
    }

    void Tex::load() {
        glGenTextures(1, &data);
        set(surface);

        if(surface != nullptr) {
            SDL_FreeSurface(surface);
            surface = nullptr;
        }
    }

    int Tex::active(int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        bind();

        return unit;
    }

    Tex2D::Tex2D(const char *filename): Tex2D(-1, -1, IMG_Load(filename)) {}
    Tex2D::Tex2D(SDL_Surface *surface): Tex2D(-1, -1, surface) {}
    Tex2D::Tex2D(int width, int height, SDL_Surface *surface): Tex(surface) {
        this->width = width;
        this->height = height;
    }

    void Tex2D::bind() {
        glBindTexture(GL_TEXTURE_2D, data);
    }

    void Tex2D::set(SDL_Surface *surface, bool bind) {
        if(surface != nullptr) {
            width = surface->w;
            height = surface->h;
        }

        SDL_Surface *substitute = nullptr;
        if(surface != nullptr && surface->format->format != SDL_PIXELFORMAT_ABGR8888) {
            substitute = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
        }

        if(bind) this->bind();
        glTexImage2D(
            GL_TEXTURE_2D, 0,
            GL_RGBA8,
            width, height, 0,
            GL_RGBA,
            GL_UNSIGNED_BYTE, substitute != nullptr ? substitute->pixels : surface != nullptr ? surface->pixels : nullptr
        );

        glGenerateMipmap(GL_TEXTURE_2D);
        if(substitute != nullptr) SDL_FreeSurface(substitute);
    }

    void Tex2D::setWrap(int s, int t, int r, bool bind) {
        if(bind) this->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t);
    }

    void Tex2D::setFilter(int min, int mag, bool bind) {
        if(bind) this->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
    }
}
