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
        data = NULL;
    }

    Tex::~Tex() {
        if(data != NULL) {
            glDeleteTextures(1, &data);
            data = NULL;
        }
    }

    void Tex::load() {
        if(surface == NULL) throw std::exception("SDL surface data is NULL.");
        
        glGenTextures(1, &data);
        set(surface);

        SDL_FreeSurface(surface);
        surface = NULL;
    }

    int Tex::active(int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        bind();

        return unit;
    }

    void Tex::setWrap(int s, int t, int r, bool bind) {
        if(bind) this->bind();
        glTexParameteri(data, GL_TEXTURE_WRAP_S, s);
        glTexParameteri(data, GL_TEXTURE_WRAP_T, t);
        glTexParameteri(data, GL_TEXTURE_WRAP_R, r);
    }

    void Tex::setFilter(int min, int mag, bool bind) {
        if(bind) this->bind();
        glTexParameteri(data, GL_TEXTURE_MIN_FILTER, min);
        glTexParameteri(data, GL_TEXTURE_MAG_FILTER, mag);
    }

    Tex2D::Tex2D(const char *filename): Tex2D(IMG_Load(filename)) {}

    Tex2D::Tex2D(SDL_Surface *surface): Tex(surface) {
        width = -1;
        height = -1;
    }

    void Tex2D::bind() {
        glBindTexture(GL_TEXTURE_2D, data);
    }

    void Tex2D::set(SDL_Surface *surface, bool bind) {
        width = surface->w;
        height = surface->h;

        if(bind) this->bind();
        glTexImage2D(
            GL_TEXTURE_2D, 0,
            GL_RGBA,
            surface->w, surface->h, 0,
            GL_RGBA,
            GL_UNSIGNED_BYTE, surface->pixels
        );
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}
