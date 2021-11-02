#ifndef ASSETS_H
#define ASSETS_H

#include <SDL.h>
#include <mutex>
#include <string>
#include <typeindex>
#include <functional>
#include <unordered_map>
#include <vector>
#include <future>

namespace Fantasy {
    class AssetManager;

    template<typename T>
    class AssetData {
        public:
        typedef T type;

        public:
        AssetData() {}
        virtual ~AssetData() {}

        void dispose() {
            this->~AssetData();
        }
    };

    template<
        typename T, typename D,
        std::enable_if<std::is_base_of<AssetData<T>, D>::value>::type *_D = nullptr
    > class AssetLoader {
        public:
        typedef T type;
        typedef D data;

        public:
        AssetLoader() {}
        virtual ~AssetLoader() {}

        virtual void loadAsync(AssetManager *, std::string, const char *, D *, SDL_RWops *) = 0;

        virtual T *loadSync(AssetManager *, std::string, const char *, D *, SDL_RWops *) = 0;

        virtual void dispose(AssetManager *, T *asset) = 0;

        void require(const char *filename, SDL_RWops *data) {
            if(data == NULL) throw std::exception(std::string("File '").append(filename).append("' does not exist.").c_str());
        }
    };

    class AssetManager {
        private:
        std::unordered_map<std::type_index, std::unordered_map<const char *, void *> *> *assets;
        std::unordered_map<std::type_index, void *> *loaders;
        std::string prefix;

        std::recursive_mutex *lock;
        std::unordered_map<const char *, std::string> *errors;
        std::vector<std::function<void()>> *syncLoads;
        std::vector<std::function<void()>> *assetDisposal;
        std::vector<std::function<void()>> *loaderDisposal;
        std::vector<std::future<void> *> *processes;

        int toLoad;
        volatile int loaded;

        public:
        AssetManager(std::string prefix = "assets/");
        ~AssetManager();

        void defaultLoaders();
        std::string getPrefix();

        bool update();
        float progress();
        void finish();
        void dispose();

        // https://stackoverflow.com/questions/456713/why-do-i-get-unresolved-external-symbol-errors-when-using-templates
        template<
            typename T, typename D,
            std::enable_if<std::is_base_of<AssetData<T>, D>::value>::type *_D = nullptr
        > void setLoader(AssetLoader<T, D> *loader) {
            lock->lock();

            std::type_index info = typeid(T);
            if(loaders->contains(info)) {
                throw std::exception(std::string("Asset loader already setup for '").append(info.name()).append("'.").c_str());
            } else {
                loaders->emplace(info, loader);
                loaderDisposal->push_back([loader]() {
                    delete loader;
                });
            }

            lock->unlock();
        }

        template<
            typename T, typename D,
            std::enable_if<std::is_base_of<AssetData<T>, D>::value>::type *_D = nullptr
        > AssetLoader<T, D> *getLoader() {
            lock->lock();
            auto loader = (AssetLoader<T, D> *)loaders->at(typeid(T));
            lock->unlock();

            return loader;
        }

        template<typename T>
        bool hasLoader() {
            lock->lock();
            bool has = loaders->contains(typeid(T));
            lock->unlock();

            return has;
        }

        template<
            typename T, typename D = AssetData<T>,
            std::enable_if<std::is_base_of<AssetData<T>, D>::value>::type *_D = nullptr
        > void load(const char *filename, std::function<void(AssetManager *, const char *, T *)> *callback, D *data = NULL) {
            if(hasAsset<T>(filename)) return;

            toLoad++;
            std::future<void> process = std::async(std::launch::async, [&]() {
                if(!hasLoader<T>()) {
                    error(filename, std::string("No loader setup for '").append(typeid(T).name()).append("'.").c_str());
                    return;
                }

                std::string str = prefix + filename;
                const char *actualName = str.c_str();

                SDL_RWops *bin = SDL_RWFromFile(actualName, "r+b");
                AssetLoader<T, D> *loader = getLoader<T, D>();
                try {
                    loader->loadAsync(this, prefix, filename, data, bin);
                } catch(std::exception &e) {
                    error(filename, std::string(e.what()));
                    return;
                }

                T *asset = NULL;

                lock->lock();
                syncLoads->push_back([&]() {
                    try {
                        asset = loader->loadSync(this, prefix, filename, data, bin);

                        (*callback)(this, filename, asset);
                        loaded++;
                    } catch(std::exception &e) {
                        if(data != NULL) data->~AssetData();
                        if(bin != NULL) SDL_RWclose(bin);

                        error(filename, std::string(e.what()));
                        return;
                    }

                    if(data != NULL) data->~AssetData();
                    if(bin != NULL) SDL_RWclose(bin);
                });

                assetDisposal->push_back([&]() {
                    if(asset != NULL) loader->dispose(this, asset);
                });
                lock->unlock();
            });

            processes->push_back(std::move(&process));
        }

        template<
            typename T, typename D = AssetData<T>,
            std::enable_if<std::is_base_of<AssetData<T>, D>::value>::type *_D = nullptr
        > void load(const char *filename, D *data = NULL) {
            load<T, D>(filename, NULL, data);
        }

        template<
            typename T, typename D = AssetData<T>,
            std::enable_if<std::is_base_of<AssetData<T>, D>::value>::type *_D = nullptr
        > void dispose(const char *filename) {
            std::type_index type = typeid(T);
            if(!hasAsset<T>(filename)) {
                throw std::exception(std::string("'").append(type.name()).append(" with name '").append(filename).append("' is not loaded.").c_str());
            } else {
                T *asset = getAsset<T>(filename);
                getLoader<T, D>()->dispose(this, asset);

                removeAsset(filename, asset);
            }
        }

        template<typename T>
        bool hasAsset(const char *name) {
            lock->lock();
            bool has = getAssets<T>()->contains(name);
            lock->unlock();

            return has;
        }

        template<typename T>
        T *getAsset(const char *name) {
            lock->lock();
            T *asset = (T *)getAssets<T>()->at(name);
            lock->unlock();

            return asset;
        }

        private:
        template<typename T>
        std::unordered_map<const char *, void *> *getAssets() {
            lock->lock();

            std::type_index type = typeid(T);
            std::unordered_map<const char *, void *> *result;
            if(assets->contains(type)) {
                result = assets->at(type);
            } else {
                assets->emplace(type, result = new std::unordered_map<const char *, void *>());
            }

            lock->unlock();
            return result;
        }

        template<typename T>
        void addAsset(const char *name, T *asset) {
            lock->lock();
            getAssets<T>()->emplace(name, asset);
            lock->unlock();
        }

        template<typename T>
        void removeAsset(const char *name, T *asset) {
            lock->lock();

            std::unordered_map<const char *, T *> *map = getAssets<T>();
            if(map->contains(name)) map->erase(map->find(name));

            lock->unlock();
        }

        void error(const char *, std::string);
    };
}

#endif
