#ifndef SHADER_H
#define SHADER_H

#include <map>
#include <SDL_opengl.h>

namespace Fantasy {
    class Shader {
        private:
        unsigned int progPtr;
        unsigned int vertPtr;
        unsigned int fragPtr;

        std::map<const char *, unsigned int> *uniforms;
        std::map<const char *, unsigned int> *attributes;

        public:
        Shader(const char *, const char *);
        Shader(char *[], char *[]);
        ~Shader();

        unsigned int uniformLoc(const char *);
        unsigned int attributeLoc(const char *);
        void bind();

        private:
        unsigned int createShader(int, char*[]);
        void logProgram();
        void logShader(unsigned int);
    };
}

#endif
