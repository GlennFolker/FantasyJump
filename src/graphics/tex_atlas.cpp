#include <string>
#include <stdexcept>
#include <fstream>

#include "tex_atlas.h"

namespace Fantasy {
    TexRegion::TexRegion(): TexRegion(nullptr, 0, 0, 1, 1) {}
    TexRegion::TexRegion(Tex2D *texture): TexRegion(texture, 0, 0, texture->width, texture->height) {}
    TexRegion::TexRegion(Tex2D *texture, int x, int y, int width, int height) {
        this->texture = texture;
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;

        if(texture == nullptr) {
            u = v2 = 0.0f;
            u2 = v = 1.0f;
        } else {
            u = (float)x / (float)texture->width;
            v2 = (float)y / (float)texture->height;
            u2 = (float)(x + width) / (float)texture->width;
            v = (float)(y + height) / (float)texture->height;
        }
    }

    TexAtlas::TexAtlas() {}
    TexAtlas::TexAtlas(const std::string &filename): TexAtlas(filename, (std::istream &&)std::move(std::ifstream(filename, std::ios::binary))) {}
    TexAtlas::TexAtlas(const std::string &filename, const std::istream &stream): TexAtlas(filename, (std::istream &&)std::move(stream)) {}
    TexAtlas::TexAtlas(const std::string &filename, std::istream &&stream) {
        char version;
        stream >> version;

        std::string prefix = filename;
        int slash = prefix.find_last_of("/");
        if(slash == std::string::npos) slash = prefix.find_last_of("\\");
        if(slash != std::string::npos) prefix = prefix.substr(0, slash + 1);
        
        switch(version) {
            case 1: {
                size_t pageSize;
                stream.read(reinterpret_cast<char *>(&pageSize), sizeof(size_t));

                for(size_t page = 0; page < pageSize; page++) {
                    size_t pageNameSize;
                    stream.read(reinterpret_cast<char *>(&pageNameSize), sizeof(size_t));

                    char pageName[pageNameSize + 1];
                    stream.read(pageName, pageNameSize);
                    pageName[pageNameSize] = '\0';

                    Tex2D *pageTex = new Tex2D(std::string(prefix).append(pageName).c_str());
                    pageTex->load();
                    pageTex->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
                    pageTex->setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
                    textures.emplace(pageTex);

                    size_t regSize;
                    stream.read(reinterpret_cast<char *>(&regSize), sizeof(size_t));

                    for(size_t reg = 0; reg < regSize; reg++) {
                        size_t regNameSize;
                        stream.read(reinterpret_cast<char *>(&regNameSize), sizeof(size_t));

                        char regName[regNameSize + 1];
                        stream.read(regName, regNameSize);
                        regName[regNameSize] = '\0';

                        int x, y, width, height;
                        stream.read(reinterpret_cast<char *>(&x), sizeof(int))
                            .read(reinterpret_cast<char *>(&y), sizeof(int))
                            .read(reinterpret_cast<char *>(&width), sizeof(int))
                            .read(reinterpret_cast<char *>(&height), sizeof(int));

                        regions.emplace(std::string(regName), TexRegion(pageTex, x, y, width, height));
                    }
                }
            } break;

            default: throw std::runtime_error(std::string("Version ").append(std::to_string(version)).append(" not supported."));
        }
    }

    TexAtlas::~TexAtlas() {
        for(const Tex *texture : textures) delete texture;
    }

    const TexRegion &TexAtlas::get(const std::string &name) const {
        if(!regions.count(name)) throw std::runtime_error(std::string("No such texture region: '").append(name).append("'."));
        return regions.at(name);
    }
}
