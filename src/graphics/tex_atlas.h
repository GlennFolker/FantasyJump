#ifndef TEX_ATLAS_H
#define TEX_ATLAS_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "tex.h"

namespace Fantasy {
    struct TexRegion {
        public:
        Tex2D *texture;
        int x, y, width, height;
        float u, v, u2, v2;

        public:
        TexRegion();
        TexRegion(Tex2D *);
        TexRegion(Tex2D *, int, int, int, int);
    };

    class TexAtlas {
        private:
        std::unordered_set<Tex *> textures;
        std::unordered_map<std::string, TexRegion> regions;

        public:
        TexAtlas();
        TexAtlas(const char *);
        TexAtlas(std::istream &&);
        TexAtlas(const std::istream &);
        ~TexAtlas();

        const TexRegion &get(const std::string &) const;
    };
}

#endif
