#include <SDL.h>

#include "sprite_batch.h"
#include "../app.h"
#include "../util/mathf.h"

#include <glm/gtc/type_ptr.hpp>

namespace Fantasy {
    SpriteBatch::SpriteBatch(): SpriteBatch(4096, NULL) {}
    
    SpriteBatch::SpriteBatch(size_t size, Shader *shader) {
        if(size > 8191) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Max vertices must not be greater than 8191.");
            this->~SpriteBatch();
            return;
        }

        z = 1.0f;
        index = 0;
        lastTexture = NULL;
        projection = identity<mat4>();

        size_t indicesCount = size * 6;
        mesh = new Mesh(size * 4, indicesCount, 2, new VertexAttr[]{
            VertexAttr::position,
            VertexAttr::texCoords
        });

        spriteSize = 0;
        for(size_t i = 0; i < mesh->attrCount; i++) {
            spriteSize += mesh->attributes[i].components;
        }
        spriteSize *= 4;

        vertLength = size * spriteSize;
        vertices = new float[vertLength];

        this->shader = shader == NULL ? new Shader(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER) : shader;

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
    }

    SpriteBatch::~SpriteBatch() {
        if(shader != NULL) shader->~Shader();
        if(mesh != NULL) mesh->~Mesh();
        if(vertices != NULL) delete[] vertices;
    }

    void SpriteBatch::draw(Tex2D *texture, float x, float y) {
        draw(texture, x, y, x - texture->width / 2.0f, y - texture->height / 2.0f, texture->width, texture->height, 0.0f);
    }

    void SpriteBatch::draw(Tex2D *texture, float x, float y, float rotation) {
        draw(texture, x, y, x - texture->width / 2.0f, y - texture->height / 2.0f, texture->width, texture->height, rotation);
    }

    void SpriteBatch::draw(Tex2D *texture, float x, float y, float originX, float originY, float rotation) {
        draw(texture, x, y, originX, originY, texture->width, texture->height, rotation);
    }

    void SpriteBatch::draw(Tex2D *texture, float x, float y, float originX, float originY, float width, float height, float rotation) {
        if(lastTexture != texture) {
            flush();
            lastTexture = texture;
        } else if(index >= vertLength) {
            flush();
        }

        vertices[index++] = originX;
        vertices[index++] = originY;
        vertices[index++] = z;
        vertices[index++] = 0.0f;
        vertices[index++] = 0.0f;

        vertices[index++] = originX + width;
        vertices[index++] = originY;
        vertices[index++] = z;
        vertices[index++] = 1.0f;
        vertices[index++] = 0.0f;

        vertices[index++] = originX + width;
        vertices[index++] = originY + height;
        vertices[index++] = z;
        vertices[index++] = 1.0f;
        vertices[index++] = 1.0f;

        vertices[index++] = originX;
        vertices[index++] = originY + height;
        vertices[index++] = z;
        vertices[index++] = 0.0f;
        vertices[index++] = 1.0f;
    }

    void SpriteBatch::proj(mat4 projection) {
        this->projection = projection;
    }

    void SpriteBatch::flush() {
        if(index == 0 || lastTexture == NULL) return;

        shader->bind();
        glUniformMatrix4fv(shader->uniformLoc("u_proj"), 1, false, value_ptr(projection));
        glUniform1i(shader->uniformLoc("u_texture"), lastTexture->active(0));
        
        mesh->setVertices(vertices, 0, index);
        mesh->render(shader, GL_TRIANGLES, 0, index / spriteSize * 6);
        index = 0;
    }
}
