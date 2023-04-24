#include <SDL.h>
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>

#include "sprite_batch.h"
#include "../app.h"
#include "../util/mathf.h"

static constexpr const char *DEFAULT_VERTEX_SHADER = R"(
#version 150 core

in vec2 a_position;
in vec2 a_tex_coords_0;
in vec4 a_color;
in vec4 a_tint;

out vec2 v_tex_coords;
out vec4 v_color;
out vec4 v_tint;

uniform mat4 u_proj;

void main() {
    gl_Position = u_proj * vec4(a_position, 1.0, 1.0);
    v_tex_coords = a_tex_coords_0;
    v_color = a_color;
    v_tint = a_tint;
})";

static constexpr const char *DEFAULT_FRAGMENT_SHADER = R"(
#version 150 core

out vec4 fragColor;

in vec2 v_tex_coords;
in vec4 v_color;
in vec4 v_tint;

uniform sampler2D u_texture;

void main() {
    vec4 base = texture2D(u_texture, v_tex_coords);
    gl_FragColor = v_color * mix(base, vec4(v_tint.rgb, base.a), v_tint.a);
})";

namespace Fantasy {
    SpriteBatch::SpriteBatch(): SpriteBatch(8191, nullptr) {}
    SpriteBatch::SpriteBatch(size_t size, Shader *shader) {
        if(size > 8191) throw std::runtime_error("Max vertices is 8191");

        color = Color::white;
        colorBits = color.fabgr();
        tinted = Color();
        tintBits = tinted.fabgr();
        index = 0;
        texture = nullptr;
        projection = glm::identity<glm::mat4>();

        size_t indicesCount = size * 6;
        mesh = new Mesh(size * 4, indicesCount, 4, new VertexAttr[4]{
            VertexAttr::position2D,
            VertexAttr::color,
            VertexAttr::tint,
            VertexAttr::texCoords
        });

        spriteSize = 4 * ( // Vertex size.
            2 + // Position.
            1 + // Base color.
            1 + // Tint color.
            2   // Texture coordinates.
        );

        vertLength = size * spriteSize;
        vertices = new float[vertLength];
        tmp = new float[spriteSize * 4];

        this->shader = shader == nullptr ? new Shader(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER) : shader;

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
        if(shader != nullptr) delete shader;
        if(mesh != nullptr) delete mesh;
        if(vertices != nullptr) delete[] vertices;
    }

    void SpriteBatch::draw(Tex2D *texture, float *vertices, size_t offset, size_t length) {
        if(length % spriteSize != 0) throw std::runtime_error(std::string("Vertices size must be increment of ").append(std::to_string(spriteSize)).c_str());

        int len = vertLength;
        int remaining = len;
        if(this->texture != texture) {
            flush();
            this->texture = texture;
        } else {
            remaining -= index;
            if(remaining == 0) {
                flush();
                remaining = len;
            }
        }

        size_t copyCount = fminf(remaining, length);

        SDL_memcpy(this->vertices + index, vertices + offset, copyCount * sizeof(float));
        index += copyCount;
        length -= copyCount;
        while(length > 0) {
            offset += copyCount;
            flush();

            copyCount = fminf(len, length);
            SDL_memcpy(this->vertices, vertices + offset, copyCount * sizeof(float));
            index += copyCount;
            length -= copyCount;
        }
    }

    void SpriteBatch::draw(const TexRegion &region, float x, float y, float rotation) {
        draw(region, x, y, x - region.width / 2.0f, y - region.height / 2.0f, region.width, region.height, rotation);
    }

    void SpriteBatch::draw(const TexRegion &region, float x, float y, float width, float height, float rotation) {
        draw(region, x, y, x - width / 2.0f, y - height / 2.0f, width, height, rotation);
    }

    void SpriteBatch::draw(const TexRegion &region, float x, float y, float originX, float originY, float width, float height, float rotation) {
        glm::mat4 trns = glm::rotate(glm::identity<glm::mat4>(), rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec2
            pos1 = trns * glm::vec4(originX - x, originY - y, 0.0f, 1.0f),
            pos2 = trns * glm::vec4(originX - x + width, originY - y, 0.0f, 1.0f),
            pos3 = trns * glm::vec4(originX - x + width, originY - y + height, 0.0f, 1.0f),
            pos4 = trns * glm::vec4(originX - x, originY - y + height, 0.0f, 1.0f);

        tmp[0] = pos1.x + x;
        tmp[1] = pos1.y + y;
        tmp[2] = colorBits;
        tmp[3] = tintBits;
        tmp[4] = region.u;
        tmp[5] = region.v;

        tmp[6] = pos2.x + x;
        tmp[7] = pos2.y + y;
        tmp[8] = colorBits;
        tmp[9] = tintBits;
        tmp[10] = region.u2;
        tmp[11] = region.v;

        tmp[12] = pos3.x + x;
        tmp[13] = pos3.y + y;
        tmp[14] = colorBits;
        tmp[15] = tintBits;
        tmp[16] = region.u2;
        tmp[17] = region.v2;

        tmp[18] = pos4.x + x;
        tmp[19] = pos4.y + y;
        tmp[20] = colorBits;
        tmp[21] = tintBits;
        tmp[22] = region.u;
        tmp[23] = region.v2;

        draw(region.texture, tmp, 0, 24);
    }

    void SpriteBatch::col(const Color &color) {
        this->color = color;
        colorBits = color.fabgr();
    }

    void SpriteBatch::col(float abgr) {
        color.fromFagbr(abgr);
        this->colorBits = abgr;
    }

    void SpriteBatch::tint(const Color &color) {
        this->tinted = color;
        tintBits = color.fabgr();
    }

    void SpriteBatch::tint(float abgr) {
        tinted.fromFagbr(abgr);
        this->tintBits = abgr;
    }

    void SpriteBatch::proj(const glm::mat4 &projection) {
        this->projection = projection;
    }

    void SpriteBatch::flush() {
        if(index == 0 || texture == nullptr) return;

        shader->bind();
        glUniformMatrix4fv(shader->uniformLoc("u_proj"), 1, false, glm::value_ptr(projection));
        glUniform1i(shader->uniformLoc("u_texture"), texture->active(0));
        
        mesh->setVertices(vertices, 0, index);
        mesh->render(shader, GL_TRIANGLES, 0, index / spriteSize * 6);
        index = 0;
    }
}
