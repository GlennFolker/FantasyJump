#include <SDL_image.h>
#include <cmath>
#include <string>

#include "tex.h"

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
        if(surface == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL surface data is NULL.");
            this->~Tex();
            return;
        }

        glGenTextures(1, &data);
        bind();
        set(surface);
        setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
    }

    int Tex::active(int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        bind();

        return unit;
    }

    void Tex::setWrap(int s, int t, int r) {
        glTexParameteri(data, GL_TEXTURE_WRAP_S, s);
        glTexParameteri(data, GL_TEXTURE_WRAP_T, t);
        glTexParameteri(data, GL_TEXTURE_WRAP_R, r);
    }

    void Tex::setFilter(int min, int mag) {
        glTexParameteri(data, GL_TEXTURE_MIN_FILTER, min);
        glTexParameteri(data, GL_TEXTURE_MAG_FILTER, mag);
    }

    Tex2D::Tex2D(SDL_Surface *surface): Tex(surface) {
        width = -1;
        height = -1;
    }

    void Tex2D::bind() {
        glBindTexture(GL_TEXTURE_2D, data);
    }

    void Tex2D::set(SDL_Surface *surface) {
        width = surface->w;
        height = surface->h;

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
