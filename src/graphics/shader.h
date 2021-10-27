#ifndef SHADER_H
#define SHADER_H

#include <SDL_opengl.h>

namespace Fantasy {
    class Shader {
        private:
        GLuint progPtr;
        GLuint vertPtr;
        GLuint fragPtr;

        public:
        Shader(const char *, const char *);
        Shader(char **, char **);
        ~Shader();

        private:
        GLuint createShader(int type, char** source);
        void logProgram();
        void logShader(GLuint);
    };
}

#endif
