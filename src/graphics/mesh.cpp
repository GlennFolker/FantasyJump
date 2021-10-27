#include <SDL.h>
#include <gl/glew.h>

#include "mesh.h"

using namespace std;

namespace Fantasy {
    const VertexAttr VertexAttr::position = VertexAttr(3, GL_FLOAT, "a_position");
    const VertexAttr VertexAttr::texCoords = VertexAttr(2, GL_FLOAT, "a_tex_coords_0");

    Mesh::Mesh(size_t maxVertices, size_t maxIndices, size_t attrCount, VertexAttr *attributes) {
        this->maxVertices = maxVertices;
        this->maxIndices = maxIndices;
        this->attrCount = attrCount;
        this->attributes = attributes;

        vertSize = 0;
        for(size_t i = 0; i < attrCount; i++) {
            VertexAttr a = attributes[i];
            vertSize += a.getSize();
        }

        vertices = new GLfloat[maxVertices * vertSize];
        indices = new GLushort[maxIndices];

        glGenBuffers(1, &verticesData);
        glBindBuffer(GL_ARRAY_BUFFER, verticesData);
        glBufferData(GL_ARRAY_BUFFER, maxVertices * vertSize * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &indicesData);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesData);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxIndices * sizeof(GLushort), indices, GL_STATIC_DRAW);
    }

    Mesh::~Mesh() {
        glDeleteBuffers(1, &verticesData);
        delete[] vertices;
        glDeleteBuffers(1, &indicesData);
        delete[] indices;

        delete[] attributes;
    }

    void Mesh::setVertices(GLfloat *vertices, size_t offset, size_t count) {
        SDL_memcpy(this->vertices, vertices + offset, count);

        glBindBuffer(GL_ARRAY_BUFFER, verticesData);
        glBufferData(GL_ARRAY_BUFFER, maxVertices * vertSize * sizeof(GLfloat), this->vertices, GL_STATIC_DRAW);
    }

    void Mesh::setIndices(GLushort *indices, size_t offset, size_t count) {
        SDL_memcpy(this->indices, indices + offset, count);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesData);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxIndices * sizeof(GLushort), this->indices, GL_STATIC_DRAW);
    }
}
