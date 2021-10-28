#include <SDL.h>

#include "../app.h"
#include "sprite_batch.h"

#include <glm/gtc/type_ptr.hpp>

namespace Fantasy {
    SpriteBatch::SpriteBatch(): SpriteBatch(4096, NULL) {}

    SpriteBatch::SpriteBatch(size_t size, Shader *shader) {
        if(size > 8191) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Max vertices must not be greater than 8191.");
            this->~SpriteBatch();
            return;
        }
        
        index = 0;

        this->shader = shader == NULL ? new Shader(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER) : shader;

        size_t indicesCount = size * 6;
        mesh = new Mesh(size * 4, indicesCount, 1, new VertexAttr[]{
            VertexAttr::position
        });
        
        spriteSize = 0;
        for(size_t i = 0; i < mesh->attrCount; i++) {
            spriteSize += mesh->attributes[i].components;
        }
        spriteSize *= 4;

        unsigned short *indices = new unsigned short[indicesCount];
        for(size_t i = 0, j = 0; i < indicesCount; i += 6, j += 4) {
            indices[i] = j;
            indices[i + 1] = j + 1;
            indices[i + 2] = j + 2;
            indices[i + 3] = j + 2;
            indices[i + 4] = j + 3;
            indices[i + 5] = j;
        }
        mesh->setIndices(indices, 0, indicesCount);
        delete[] indices;

        vertices = new float[size * spriteSize];
    }

    SpriteBatch::~SpriteBatch() {
        if(shader != NULL) shader->~Shader();
        if(mesh != NULL) mesh->~Mesh();
        if(vertices != NULL) delete[] vertices;
    }

    void SpriteBatch::flush(mat4 projection) {
        shader->bind();
        glUniformMatrix4fv(shader->uniformLoc("u_proj"), 1, false, value_ptr(projection));

        mesh->setVertices(vertices, 0, index);
        mesh->render(shader, GL_TRIANGLES, 0, index / spriteSize * 6);
        index = 0;
    }
}
