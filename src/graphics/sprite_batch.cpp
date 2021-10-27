#include <SDL.h>

#include "../app.h"
#include "sprite_batch.h"

#include <glm/gtx/transform.hpp>

namespace Fantasy {
    SpriteBatch::SpriteBatch(): SpriteBatch(4096, NULL) {}

    SpriteBatch::SpriteBatch(size_t size, Shader *shader) {
        if(size > 8191) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Max vertices must not be greater than 8191.");
            this->~SpriteBatch();
            return;
        }
        
        batching = false;
        index = 0;
        projection = ortho(0, App::instance->getWidth(), 0, App::instance->getHeight());
        transform = identity<mat4>();

        this->shader = shader == NULL ? new Shader(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER) : shader;

        size_t indicesCount = size * 6;
        mesh = new Mesh(size * 4, indicesCount, 1, new VertexAttr[]{
            VertexAttr::position
        });

        GLushort *indices = new GLushort[indicesCount];
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

        vertices = new GLfloat[size * (4 * 3)];
    }

    SpriteBatch::~SpriteBatch() {
        if(shader != NULL) shader->~Shader();
        if(mesh != NULL) mesh->~Mesh();
        if(vertices != NULL) delete[] vertices;
    }

    void SpriteBatch::begin() {
        if(batching) throw std::exception("Don't begin() twice.");
        batching = true;

        vertices[index++] = -10.0f;
        vertices[index++] = -10.0f;
        vertices[index++] = 0.0f;

        vertices[index++] = 10.0f;
        vertices[index++] = -10.0f;
        vertices[index++] = 0.0f;

        vertices[index++] = 10.0f;
        vertices[index++] = 10.0f;
        vertices[index++] = 0.0f;

        vertices[index++] = -10.0f;
        vertices[index++] = 10.0f;
        vertices[index++] = 0.0f;
    }

    void SpriteBatch::end() {
        if(!batching) throw std::exception("Don't end() twice.");
        batching = false;

        shader->bind();
        glUniformMatrix4fv(shader->uniformLoc("u_proj"), 1, false, &projection[0][0]);
        glUniformMatrix4fv(shader->uniformLoc("u_trans"), 1, false, &transform[0][0]);

        mesh->setVertices(vertices, 0, index);
        mesh->render(shader, GL_TRIANGLES, 0, index / (4 * 3) * 6);
        index = 0;
    }
}
