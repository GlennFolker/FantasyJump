#include <SDL.h>

#include "sprite_batch.h"
#include "../app.h"
#include "../util/mathf.h"

#include <glm/gtc/type_ptr.hpp>

namespace Fantasy {
    SpriteBatch::SpriteBatch(): SpriteBatch(4096, NULL) {}
    
    SpriteBatch::SpriteBatch(size_t size, Shader *shader) {
        if(size > 8191) throw std::exception("Max vertices is 8191");

        z = 1.0f;
        color = Color(1.0f, 1.0f, 1.0f, 1.0f);
        colorBits = color.fabgr();
        index = 0;
        texture = NULL;
        projection = identity<mat4>();

        size_t indicesCount = size * 6;
        mesh = new Mesh(size * 4, indicesCount, 3, new VertexAttr[]{
            VertexAttr::position,
            VertexAttr::color,
            VertexAttr::texCoords
        });

        spriteSize = 4 * (3 + 1 + 2);

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
        if(shader != NULL) delete shader;
        if(mesh != NULL) delete mesh;
        if(vertices != NULL) delete[] vertices;
    }

    void SpriteBatch::draw(Tex2D *texture, float x, float y) {
        draw(texture, x, y, x - texture->width / 2.0f, y - texture->height / 2.0f, texture->width, texture->height, 0.0f);
    }

    void SpriteBatch::draw(Tex2D *texture, float x, float y, float rotation) {
        draw(texture, x, y, x - texture->width / 2.0f, y - texture->height / 2.0f, texture->width, texture->height, rotation);
    }

    void SpriteBatch::draw(Tex2D *texture, float x, float y, float width, float height, float rotation) {
        draw(texture, x, y, x - width / 2.0f, y - height / 2.0f, width, height, rotation);
    }

    void SpriteBatch::draw(Tex2D *texture, float x, float y, float originX, float originY, float width, float height, float rotation) {
        if(this->texture != texture) {
            flush();
            this->texture = texture;
        } else if(index >= vertLength) {
            flush();
        }

        mat4 trns = rotate(identity<mat4>(), radians(rotation), vec3(0.0f, 0.0f, 1.0f));
        vec2
            pos1 = trns * vec4(originX - x, originY - y, 0.0f, 1.0f),
            pos2 = trns * vec4(originX - x + width, originY - y, 0.0f, 1.0f),
            pos3 = trns * vec4(originX - x + width, originY - y + height, 0.0f, 1.0f),
            pos4 = trns * vec4(originX - x, originY - y + height, 0.0f, 1.0f);

        vertices[index++] = pos1.x + x;
        vertices[index++] = pos1.y + y;
        vertices[index++] = z;
        vertices[index++] = colorBits;
        vertices[index++] = 0.0f;
        vertices[index++] = 1.0f;

        vertices[index++] = pos2.x + x;
        vertices[index++] = pos2.y + y;
        vertices[index++] = z;
        vertices[index++] = colorBits;
        vertices[index++] = 1.0f;
        vertices[index++] = 1.0f;

        vertices[index++] = pos3.x + x;
        vertices[index++] = pos3.y + y;
        vertices[index++] = z;
        vertices[index++] = colorBits;
        vertices[index++] = 1.0f;
        vertices[index++] = 0.0f;

        vertices[index++] = pos4.x + x;
        vertices[index++] = pos4.y + y;
        vertices[index++] = z;
        vertices[index++] = colorBits;
        vertices[index++] = 0.0f;
        vertices[index++] = 0.0f;
    }

    void SpriteBatch::col(Color color) {
        this->color = color;
        colorBits = color.fabgr();
    }

    void SpriteBatch::col(float abgr) {
        color.fromFagbr(abgr);
        this->colorBits = abgr;
    }

    void SpriteBatch::proj(mat4 projection) {
        this->projection = projection;
    }

    void SpriteBatch::flush() {
        if(index == 0 || texture == NULL) return;

        shader->bind();
        glUniformMatrix4fv(shader->uniformLoc("u_proj"), 1, false, value_ptr(projection));
        glUniform1i(shader->uniformLoc("u_texture"), texture->active(0));
        
        mesh->setVertices(vertices, 0, index);
        mesh->render(shader, GL_TRIANGLES, 0, index / spriteSize * 6);
        index = 0;
    }
}
