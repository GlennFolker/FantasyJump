#include <SDL.h>

#include "../app.h"
#include "sprite_batch.h"

#include <glm/gtx/transform.hpp>

namespace Fantasy {
    Shader *SpriteBatch::defaultShader = NULL;

    SpriteBatch::SpriteBatch(): SpriteBatch(4096, NULL) {}

    SpriteBatch::SpriteBatch(size_t maxVertices, Shader *shader) {
        if(maxVertices > 8191) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Max vertices must not be greater than 8191.");
            this->~SpriteBatch();
            return;
        }
        
        batching = false;
        index = 0;
        projection = ortho(0, App::instance->getWidth(), 0, App::instance->getHeight());
        transform = identity<mat4>();

        if(shader == NULL) {
            if(defaultShader == NULL) defaultShader = new Shader(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER);
            this->shader = defaultShader;
        } else {
            this->shader = shader;
        }

        mesh = new Mesh(maxVertices * 4, maxVertices * 6, 2, new VertexAttr[]{
            VertexAttr::position,
            VertexAttr::texCoords
        });
    }

    SpriteBatch::~SpriteBatch() {
        if(shader != NULL) shader->~Shader();
        if(mesh != NULL) mesh->~Mesh();
    }

    void SpriteBatch::begin() {
        if(batching) throw std::exception("Don't begin() twice.");
        batching = true;
    }

    void SpriteBatch::end() {
        if(!batching) throw std::exception("Don't end() twice.");
        batching = false;
    }
}
