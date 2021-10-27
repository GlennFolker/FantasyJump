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
        Shader(char **, char **);
        ~Shader();

        private:
        GLuint createShader(int type, char** source);
    };
}

#endif
