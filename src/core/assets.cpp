#include <exception>

#include "assets.h"
#include "../assets/tex_loader.h"

namespace Fantasy {
    AssetManager::AssetManager(std::string prefix) {
        this->prefix = prefix;
        if(!this->prefix.ends_with("/")) {
            this->prefix.append("/");
        }

        assets = new std::unordered_map<std::type_index, std::unordered_map<const char *, void *> *>();
        loaders = new std::unordered_map<std::type_index, void *>();
        errors = new std::unordered_map<const char *, std::string>();
        syncLoads = new std::vector<std::function<void()>>();
        assetDisposal = new std::vector<std::function<void()>>();
        loaderDisposal = new std::vector<std::function<void()>>();

        loaded = 0;
        toLoad = 0;
        lock = new std::recursive_mutex();
    }

    AssetManager::~AssetManager() {
        dispose();
        syncLoads->~vector();
        assetDisposal->~vector();
        loaderDisposal->~vector();
        assets->~unordered_map();
        loaders->~unordered_map();
        errors->~unordered_map();
        prefix.~basic_string();
        lock->~recursive_mutex();
    }
    
    void AssetManager::defaultLoaders() {
        setLoader<Tex2D, Tex2DData>(new Tex2DLoader());
    }

    std::string AssetManager::getPrefix() {
        return prefix;
    }

    void AssetManager::error(const char *filename, std::string error) {
        lock->lock();
        errors->emplace(filename, error);
        lock->unlock();
    }

    bool AssetManager::update() {
        lock->lock();
        if(!errors->empty()) {
            std::string msg;
            for(auto e = errors->begin(); e != errors->end(); ++e) {
                if(e != errors->begin()) msg.append("\n");
                msg.append("Error while loading '").append(e->first).append("': ").append(e->second);
            }

            errors->clear();
            lock->unlock();

            throw std::exception(msg.c_str());
        }

        syncLoads->erase(std::remove_if(
            syncLoads->begin(), syncLoads->end(),
            [](std::function<void()> &func) {
                func();
                return true;
            }
        ), syncLoads->end());

        lock->unlock();
        return errors->empty() && loaded >= toLoad;
    }

    void AssetManager::dispose() {
        for(auto f : *assetDisposal) f();
        for(auto f : *loaderDisposal) f();
        for(auto map : *assets) map.second->~unordered_map();
        loaders->clear();
        assets->clear();
        errors->clear();
    }

    void AssetManager::finish() {
        while(!update());
    }

    float AssetManager::progress() {
        return fminf(1.0f, (float)loaded / (float)toLoad);
    }
}
