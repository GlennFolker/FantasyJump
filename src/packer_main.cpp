#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <math.h>
#include <optional>
#include <unordered_map>
#include <vector>

#include "util/pngio.h"

namespace fs = std::filesystem;
using namespace Fantasy;

struct Bound {
    int x;
    int y;
    int width;
    int height;
};

struct Node {
    Bound rect;

    bool full;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    Node() {
        full = false;
        left = std::shared_ptr<Node>(NULL);
        right = std::shared_ptr<Node>(NULL);
    }
};

struct Page {
    std::shared_ptr<Node> root;
    std::unordered_map<const Png *, Bound> rects;

    Page(int padding, int width, int height) {
        root = std::make_shared<Node>();
        root->rect = {padding, padding, width - padding * 2, height - padding * 2};
    }
};

const std::optional<std::shared_ptr<Node>> insert(const std::shared_ptr<Node> &ptr, Bound &rect) {
    Node &node = *ptr.get();
    if(!node.full && node.left && node.right) {
        auto &child = insert(node.left, rect);
        if(!child.has_value()) return insert(node.right, rect);

        return child;
    } else {
        if(node.full) return {};
        if(node.rect.width == rect.width && node.rect.height == rect.height) return ptr;
        if(node.rect.width < rect.width || node.rect.height < rect.height) return {};

        node.left = std::make_shared<Node>();
        node.right = std::make_shared<Node>();

        int deltaWidth = node.rect.width - rect.width;
        int deltaHeight = node.rect.height - rect.height;
        if(deltaWidth > deltaHeight) {
            node.left->rect.x = node.rect.x;
            node.left->rect.y = node.rect.y;
            node.left->rect.width = rect.width;
            node.left->rect.height = node.rect.height;

            node.right->rect.x = node.rect.x + rect.width;
            node.right->rect.y = node.rect.y;
            node.right->rect.width = node.rect.width - rect.width;
            node.right->rect.height = node.rect.height;
        } else {
            node.left->rect.x = node.rect.x;
            node.left->rect.y = node.rect.y;
            node.left->rect.width = node.rect.width;
            node.left->rect.height = rect.height;

            node.right->rect.x = node.rect.x;
            node.right->rect.y = node.rect.y + rect.height;
            node.right->rect.width = node.rect.width;
            node.right->rect.height = node.rect.height - rect.height;
        }

        return insert(node.left, rect);
    }
}

int main(int argc, char *argv[]) {
    fs::path root(fs::current_path());
    fs::path target(fs::current_path());

    int maxWidth = 4096, maxHeight = 4096, padding = 4;

    if(argc > 1) target.append(argv[1]);
    if(argc > 2) maxWidth = maxHeight = std::stoi(argv[2]);
    if(argc > 3) maxHeight = std::stoi(argv[3]);
    if(argc > 4) padding = std::stoi(argv[4]);

    if(!fs::exists(target) || !fs::is_directory(target)) {
        printf("Invalid directory %s\n", target.string().c_str());
        return 1;
    }

    printf("Packing textures in %s...\n", target.string().c_str());
    std::vector<Png *> textures;
    
    fs::recursive_directory_iterator it(target);
    for(const fs::directory_entry &child : it) {
        std::string cpath = child.path().string();
        if(cpath.length() >= 4 && cpath.compare(cpath.length() - 4, 4, ".png") == 0) {
            textures.push_back(new Png(cpath.c_str()));
        }
    }

    bool errored = false;
    for(const Png *texture : textures) {
        if(texture->errored) {
            errored = true;
            break;
        }
    }

    if(errored) {
        for(const Png *texture : textures) delete texture;
        return 1;
    }

    printf("Packing %d texture%s...\nMax width : %d\nMax height: %d\nPadding: %d\n", textures.size(), textures.size() != 1 ? "s" : "", maxWidth, maxHeight, padding);
    
    std::sort(textures.begin(), textures.end(), [](const Png *a, const Png *b) -> bool {
        return fmax(a->width, a->height) > fmax(b->width, b->height);
    });

    std::vector<Page> pages;
    for(const Png *texture : textures) {
        Page &page = [&]()-> Page & {
            if(pages.empty()) pages.push_back(Page(padding, maxWidth, maxHeight));
            return pages.at(pages.size() - 1);
        }();

        Bound rect = {0, 0, texture->width + padding, texture->height + padding};
        if(rect.width + padding * 2 > maxWidth || rect.height + padding * 2 > maxHeight) {
            printf("Texture atlas not big enough.\n");

            for(const Png *texture : textures) delete texture;
            return 1;
        }

        auto node = insert(page.root, rect);
        if(!node.has_value()) {
            pages.push_back(Page(padding, maxWidth, maxHeight));
            node = insert(pages.at(pages.size() - 1).root, rect);
        }

        node.value()->full = true;
        rect = node.value()->rect;
        rect.width -= padding;
        rect.height -= padding;
        
        page.rects.emplace(texture, rect);
    }

    printf("Generating %d page%s...\n", pages.size(), pages.size() != 1 ? "s" : "");

    std::ofstream out(fs::path(root).append("texture.atlas").string().c_str(), std::ios::binary);
    if(!out.is_open()) {
        for(const Png *texture : textures) delete texture;

        printf("Couldn't open output file.\n");
        return 1;
    }
    
    const std::string &targetstr = target.string();
    int num = 0;

    out << (char)1; // Atlas version.
    
    size_t size = pages.size();
    out.write(reinterpret_cast<char *>(&size), sizeof(size_t));
    for(const Page &page : pages) {
        Png atlas(maxWidth, maxHeight);

        std::string pagestr;
        pagestr.append("texture").append(std::to_string(num++)).append(".png");

        size = pagestr.size();
        out.write(reinterpret_cast<char *>(&size), sizeof(size_t));
        out.write(pagestr.c_str(), size);

        size = page.rects.size();
        out.write(reinterpret_cast<char *>(&size), sizeof(size_t));
        for(const std::pair<const Png *, Bound> &e : page.rects) {
            const Png &png = *e.first;
            const Bound &rect = e.second;

            std::string namestr = png.filename.substr(targetstr.length() + 1);
            namestr = namestr.substr(0, namestr.length() - 4);

            const char *name = namestr.c_str();
            size = strlen(name);

            out.write(reinterpret_cast<char *>(&size), sizeof(size_t)).write(name, size)
                .write(reinterpret_cast<const char *>(&rect.x), sizeof(int))
                .write(reinterpret_cast<const char *>(&rect.y), sizeof(int))
                .write(reinterpret_cast<const char *>(&rect.width), sizeof(int))
                .write(reinterpret_cast<const char *>(&rect.height), sizeof(int));

            for(int y = 0; y < rect.height; y++) {
                int ty = y + rect.y;
                for(int x = 0; x < rect.width; x++) {
                    int tx = 4 * (x + rect.x);
                    for(int i = 0; i < 4; i++) atlas.data[ty][tx + i] = png.data[y][4 * x + i];
                }
            }
        }

        atlas.write(fs::path(root).append(pagestr.c_str()).string().c_str());
    }

    out.close();
    for(const Png *texture : textures) delete texture;
    
    printf("Done generating.\n");
    return 0;
}
