#ifndef MESH_H
#define MESH_H

#include <gl/glew.h>

namespace Fantasy {
    struct VertexAttr {
        int components;
        int size;
    };

    class Mesh {
        public:
        GLuint verticesData;
        GLuint indicesData;
        GLfloat *vertices;
        GLushort *indices;

        VertexAttr *attributes;
        size_t maxVertices;
        size_t maxIndices;
        size_t vertSize;
        size_t attrCount;

        public:
        Mesh(size_t, size_t, size_t, VertexAttr *);
        ~Mesh();

        void setVertices(GLfloat *, size_t, size_t);
        void setIndices(GLushort *, size_t, size_t);
    };
}

#endif
