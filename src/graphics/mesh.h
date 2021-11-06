#ifndef MESH_H
#define MESH_H

#include <exception>
#include <gl/glew.h>

#include "shader.h"

namespace Fantasy {
    struct VertexAttr {
        public:
        static const VertexAttr position;
        static const VertexAttr position2D;
        static const VertexAttr texCoords;
        static const VertexAttr color;
        static const VertexAttr tint;

        int components;
        int type;
        bool normalized;
        const char *alias;
        int size;

        public:
        VertexAttr(int components, int type, const char *alias): VertexAttr(components, type, false, alias) {}

        VertexAttr(int, int, bool, const char *);
    };

    class Mesh {
        public:
        size_t maxVertices;
        size_t maxIndices;
        size_t vertSize;

        size_t attrCount;
        VertexAttr *attributes;

        private:
        float *vertices;
        unsigned short *indices;
        unsigned int verticesData;
        unsigned int indicesData;

        public:
        Mesh(size_t, size_t, size_t, VertexAttr *);
        ~Mesh();

        void setVertices(float *, size_t, size_t);
        void setIndices(unsigned short *, size_t, size_t);
        void render(Shader *, unsigned int, size_t, size_t);
        void bind(Shader *);
        void unbind(Shader *);
    };
}

#endif
