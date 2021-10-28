#include <SDL.h>
#include <gl/glew.h>
#include <string>

#include "mesh.h"

using namespace std;

namespace Fantasy {
    const VertexAttr VertexAttr::position = VertexAttr(3, GL_FLOAT, "a_position");
    const VertexAttr VertexAttr::texCoords = VertexAttr(2, GL_FLOAT, "a_tex_coords_0");

    VertexAttr::VertexAttr(int components, int type, bool normalized, const char *alias) {
        this->components = components;
        this->type = type;
        this->normalized = normalized;
        this->alias = alias;

        switch(type) {
            case GL_FLOAT:
            case GL_FIXED:
                size = sizeof(float) * components;
                break;
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
                size = components;
                break;
            case GL_UNSIGNED_SHORT:
            case GL_SHORT:
                size = sizeof(short) * components;
                break;
            default:
                throw std::exception("Invalid attribute type.");
        }
    }

    Mesh::Mesh(size_t maxVertices, size_t maxIndices, size_t attrCount, VertexAttr *attributes) {
        this->maxVertices = maxVertices;
        this->maxIndices = maxIndices;
        this->attrCount = attrCount;
        this->attributes = attributes;

        vertSize = 0;
        for(size_t i = 0; i < attrCount; i++) {
            VertexAttr a = attributes[i];
            vertSize += a.size;
        }

        vertices = new float[maxVertices * vertSize];
        indices = new unsigned short[maxIndices];

        glGenBuffers(1, &verticesData);
        glBindBuffer(GL_ARRAY_BUFFER, verticesData);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, NULL);

        glGenBuffers(1, &indicesData);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesData);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
    }

    Mesh::~Mesh() {
        delete[] attributes;

        glDeleteBuffers(1, &verticesData);
        verticesData = NULL;
        delete[] vertices;

        glDeleteBuffers(1, &indicesData);
        indicesData = NULL;
        delete[] indices;
    }

    void Mesh::setVertices(float *vertices, size_t offset, size_t count) {
        SDL_memcpy(this->vertices, vertices + offset, count * sizeof(float));

        glBindBuffer(GL_ARRAY_BUFFER, verticesData);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), this->vertices, GL_STATIC_DRAW);
    }

    void Mesh::setIndices(unsigned short *indices, size_t offset, size_t count) {
        SDL_memcpy(this->indices, indices + offset, count * sizeof(unsigned short));
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesData);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned short), this->indices, GL_STATIC_DRAW);
    }

    void Mesh::render(Shader *shader, unsigned int type, size_t offset, size_t count) {
        bind(shader);
        glDrawElements(type, count, GL_UNSIGNED_SHORT, (void *)offset);
        unbind(shader);
    }

    void Mesh::bind(Shader *shader) {
        glBindBuffer(GL_ARRAY_BUFFER, verticesData);

        size_t off = 0;
        for(size_t i = 0; i < attrCount; i++) {
            VertexAttr attr = attributes[i];
            unsigned int loc = shader->attributeLoc(attr.alias);

            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, attr.components, attr.type, attr.normalized, vertSize, (void *)off);

            off += attr.size;
        }
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesData);
    }

    void Mesh::unbind(Shader *shader) {
        for(size_t i = 0; i < attrCount; i++) {
            glDisableVertexAttribArray(shader->attributeLoc(attributes[i].alias));
        }

        glBindBuffer(GL_ARRAY_BUFFER, NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
    }
}
