#include "tex_loader.h"

#include <SDL.h>
#include <gl/glew.h>

namespace Fantasy {
    template<typename T>
    TexData<T>::TexData() {
        minFilter = GL_NEAREST_MIPMAP_NEAREST;
        magFilter = GL_NEAREST_MIPMAP_NEAREST;
        wrapS = GL_CLAMP_TO_EDGE;
        wrapT = GL_CLAMP_TO_EDGE;
        wrapR = GL_CLAMP_TO_EDGE;
    }

    Tex2D *Tex2DLoader::load(AssetManager *assets, std::string prefix, const char *filename, Tex2DData *data, SDL_RWops *file) {
        require(filename, file);
        Tex2D *tex = new Tex2D(IMG_Load_RW(file, false));
        
        if(data != NULL) {
            tex->bind();
            tex->setFilter(data->minFilter, data->magFilter, false);
            tex->setWrap(data->wrapS, data->wrapT, data->wrapR, false);
        }

        return tex;
    }

    void Tex2DLoader::dispose(AssetManager *, Tex2D *asset) {
        asset->~Tex2D();
    }
}
