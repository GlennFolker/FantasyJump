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
                    size = sizeof(GLfloat) * components;
                    break;
                case GL_UNSIGNED_BYTE:
                case GL_BYTE:
                    size = sizeof(GLbyte) * components;
                    break;
                case GL_UNSIGNED_SHORT:
                case GL_SHORT:
                    size = sizeof(GLshort) * components;
                    break;
                default:
                    throw std::exception("Invalid attribute type.");
            }
        }
    };

    class Mesh {
        public:
        GLuint verticesData;
        GLuint indicesData;

        GLfloat *vertices;
        GLushort *indices;

        size_t maxVertices;
        size_t maxIndices;
        size_t vertSize;

        size_t attrCount;
        VertexAttr *attributes;

        public:
        Mesh(size_t, size_t, size_t, VertexAttr *);
        ~Mesh();

        void setVertices(GLfloat *, size_t, size_t);
        void setIndices(GLushort *, size_t, size_t);
        void render(Shader *, GLenum, size_t, size_t);
        void bind(Shader *);
        void unbind(Shader *);
    };
}

#endif
