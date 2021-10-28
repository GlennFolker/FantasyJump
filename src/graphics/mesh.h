#ifndef MESH_H
#define MESH_H

#include <exception>
#include <gl/glew.h>

#include "shader.h"

namespace Fantasy {
    struct VertexAttr {
        public:
        static const VertexAttr position;
        static const VertexAttr texCoords;

        int components;
        int type;
        bool normalized;
        const char *alias;
        int size;

        public:
        VertexAttr(int components, int type, const char *alias): VertexAttr(components, type, false, alias) {}

        VertexAttr(int components, int type, bool normalized, const char *alias) {
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
    };

    class Mesh {
        public:
        unsigned int verticesData;
        unsigned int indicesData;

        float *vertices;
        unsigned short *indices;

        size_t maxVertices;
        size_t maxIndices;
        size_t vertSize;

        size_t attrCount;
        VertexAttr *attributes;

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
