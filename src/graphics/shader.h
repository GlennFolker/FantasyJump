#ifndef SHADER_H
#define SHADER_H

#include <unordered_map>
#include <SDL_opengl.h>

namespace Fantasy {
    class Shader {
        private:
        unsigned int progPtr;
        unsigned int vertPtr;
        unsigned int fragPtr;

        std::unordered_map<const char *, unsigned int> *uniforms;
        std::unordered_map<const char *, unsigned int> *attributes;

        public:
        Shader(const char *, const char *);
        Shader(const char **, const char **);
        ~Shader();

        unsigned int uniformLoc(const char *);
        unsigned int attributeLoc(const char *);
        void bind();

        private:
        unsigned int createShader(int, const char**);
        void logProgram();
        void logShader(unsigned int);
    };
}

#endif
