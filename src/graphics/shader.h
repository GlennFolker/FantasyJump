#ifndef SHADER_H
#define SHADER_H

#include <map>
#include <SDL_opengl.h>

namespace Fantasy {
    class Shader {
        private:
        GLuint progPtr;
        GLuint vertPtr;
        GLuint fragPtr;

        std::map<const char *, GLuint> *uniforms;
        std::map<const char *, GLuint> *attributes;

        public:
        Shader(const char *, const char *);
        Shader(char *[], char *[]);
        ~Shader();

        GLuint uniformLoc(const char *);
        GLuint attributeLoc(const char *);
        void bind();

        private:
        GLuint createShader(int, char*[]);
        void logProgram();
        void logShader(GLuint);
    };
}

#endif
