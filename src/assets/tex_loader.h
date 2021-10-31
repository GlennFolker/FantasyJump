#ifndef TEX_LOADER_H
#define TEX_LOADER_H

#include "../core/assets.h"
#include "../graphics/tex.h"

namespace Fantasy {
    template<typename T>
    class TexData: public AssetData<T> {
        public:
        int minFilter;
        int magFilter;
        int wrapS;
        int wrapT;
        int wrapR;

        public:
        TexData();
    };

    class Tex2DData: public TexData<Tex2D> {};

    class Tex2DLoader: public AssetLoader<Tex2D, Tex2DData> {
        public:
        Tex2D *load(AssetManager *, std::string, const char *, Tex2DData *, SDL_RWops *) override;

        void dispose(AssetManager *, Tex2D *asset) override;
    };
}

#endif
