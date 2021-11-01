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

    Tex2DLoader::Tex2DLoader() {
        lock = new std::recursive_mutex();
        datas = new std::unordered_map<const char *, Tex2D *>();
    }

    Tex2DLoader::~Tex2DLoader() {
        datas->~unordered_map();
        lock->~recursive_mutex();
    }

    void Tex2DLoader::loadAsync(AssetManager *assets, std::string prefix, const char *filename, Tex2DData *data, SDL_RWops *file) {
        require(filename, file);
        Tex2D *tex = new Tex2D(IMG_Load_RW(file, false));

        lock->lock();
        datas->emplace(filename, tex);
        lock->unlock();
    }

    Tex2D *Tex2DLoader::loadSync(AssetManager *assets, std::string prefix, const char *filename, Tex2DData *data, SDL_RWops *file) {
        lock->lock();

        Tex2D *tex = datas->at(filename);
        datas->erase(datas->find(filename));

        lock->unlock();

        tex->load();
        if(data != NULL) {
            tex->setFilter(data->minFilter, data->magFilter, false);
            tex->setWrap(data->wrapS, data->wrapT, data->wrapR, false);
        }

        return tex;
    }

    void Tex2DLoader::dispose(AssetManager *, Tex2D *asset) {
        asset->~Tex2D();
    }
}
