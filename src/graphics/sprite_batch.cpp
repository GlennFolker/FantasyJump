#include <SDL.h>
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>

#include "sprite_batch.h"
#include "../app.h"
#include "../util/mathf.h"

constexpr const char *DEFAULT_VERTEX_SHADER = R"(
#version 150 core

in vec3 a_position;
in vec2 a_tex_coords_0;
in vec4 a_color;
in vec4 a_tint;

out vec2 v_tex_coords;
out vec4 v_color;
out vec4 v_tint;

uniform mat4 u_proj;

void main() {
    gl_Position = u_proj * vec4(a_position, 1.0);
    v_tex_coords = a_tex_coords_0;
    v_color = a_color;
    v_tint = a_tint;
}
)";

constexpr const char *DEFAULT_FRAGMENT_SHADER = R"(
#version 150 core

out vec4 fragColor;

in vec2 v_tex_coords;
in vec4 v_color;
in vec4 v_tint;

uniform sampler2D u_texture;

void main() {
    vec4 base = texture2D(u_texture, v_tex_coords);
    gl_FragColor = v_color * mix(base, vec4(v_tint.rgb, base.a), v_tint.a);
}
)";

namespace Fantasy {
    SpriteBatch::SpriteBatch(): SpriteBatch(8191, NULL) {}
    SpriteBatch::SpriteBatch(size_t size, Shader *shader) {
        if(size > 8191) throw std::runtime_error("Max vertices is 8191");

        z = 1.0f;
        color = Color::white;
        colorBits = color.fabgr();
        tinted = Color();
        tintBits = tinted.fabgr();
        index = 0;
        texture = NULL;
        projection = identity<mat4>();

        size_t indicesCount = size * 6;
        mesh = new Mesh(size * 4, indicesCount, 4, new VertexAttr[4]{
            VertexAttr::position,
            VertexAttr::color,
            VertexAttr::tint,
            VertexAttr::texCoords
        });

        spriteSize = 4 * ( // Vertex size.
            3 + // Position.
            1 + // Base color.
            1 + // Tint color.
            2   // Texture coordinates.
        );

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

        mat4 trns = rotate(identity<mat4>(), rotation, vec3(0.0f, 0.0f, 1.0f));
        vec2
            pos1 = trns * vec4(originX - x, originY - y, 0.0f, 1.0f),
            pos2 = trns * vec4(originX - x + width, originY - y, 0.0f, 1.0f),
            pos3 = trns * vec4(originX - x + width, originY - y + height, 0.0f, 1.0f),
            pos4 = trns * vec4(originX - x, originY - y + height, 0.0f, 1.0f);

        vertices[index++] = pos1.x + x;
        vertices[index++] = pos1.y + y;
        vertices[index++] = z;
        vertices[index++] = colorBits;
        vertices[index++] = tintBits;
        vertices[index++] = 0.0f;
        vertices[index++] = 1.0f;

        vertices[index++] = pos2.x + x;
        vertices[index++] = pos2.y + y;
        vertices[index++] = z;
        vertices[index++] = colorBits;
        vertices[index++] = tintBits;
        vertices[index++] = 1.0f;
        vertices[index++] = 1.0f;

        vertices[index++] = pos3.x + x;
        vertices[index++] = pos3.y + y;
        vertices[index++] = z;
        vertices[index++] = colorBits;
        vertices[index++] = tintBits;
        vertices[index++] = 1.0f;
        vertices[index++] = 0.0f;

        vertices[index++] = pos4.x + x;
        vertices[index++] = pos4.y + y;
        vertices[index++] = z;
        vertices[index++] = colorBits;
        vertices[index++] = tintBits;
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

    void SpriteBatch::tint(Color color) {
        this->tinted = color;
        tintBits = color.fabgr();
    }

    void SpriteBatch::tint(float abgr) {
        tinted.fromFagbr(abgr);
        this->tintBits = abgr;
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
