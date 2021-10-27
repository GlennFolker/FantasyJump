#ifndef MESH_H
#define MESH_H

#include <gl/glew.h>

namespace Fantasy {
    struct VertexAttr {
        public:
        static const VertexAttr position;
        static const VertexAttr texCoords;

        private:
        int components;
        int type;
        bool normalized;
        char *alias;
        int size;

        public:
        VertexAttr(int components, int type, const char *alias): VertexAttr(components, type, false, alias) {}

        VertexAttr(int components, int type, bool normalized, const char *alias) {
            this->components = components;
            this->type = type;
            this->normalized = normalized;
            this->alias = (char *)alias;

            switch(type) {
                case GL_FLOAT:
                case GL_FIXED:
                    size = 4 * components;
                    break;
                case GL_UNSIGNED_BYTE:
                case GL_BYTE:
                    size = components;
                    break;
                case GL_UNSIGNED_SHORT:
                case GL_SHORT:
                    size = 2 * components;
                    break;
            }
        }

        int getComponents() {
            return components;
        }

        int getType() {
            return type;
        }

        bool isNormalized() {
            return normalized;
        }

        char *getAlias() {
            return alias;
        }

        int getSize() {
            return size;
        }
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
