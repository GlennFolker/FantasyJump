#include <SDL.h>
#include <gl/glew.h>
#include <string>

#include "shader.h"

namespace Fantasy {
    Shader::Shader(const char *vertSource, const char *fragSource): Shader(&vertSource, &fragSource) {}

    Shader::Shader(const char **vertSource, const char **fragSource) {
        vertPtr = createShader(GL_VERTEX_SHADER, vertSource);
        if(vertPtr == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create vertex shader.");
            this->~Shader();
            return;
        }

        fragPtr = createShader(GL_FRAGMENT_SHADER, fragSource);
        if(fragPtr == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create fragment shader.");
            this->~Shader();
            return;
        }

        progPtr = glCreateProgram();
        if(progPtr == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create GL program.");
            this->~Shader();
            return;
        }
        
        glAttachShader(progPtr, vertPtr);
        glAttachShader(progPtr, fragPtr);
        glBindFragDataLocation(progPtr, 0, "fragColor");
        glLinkProgram(progPtr);

        int success;
        glGetProgramiv(progPtr, GL_LINK_STATUS, &success);
        if(success != GL_TRUE) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't link shader program.");
            logProgram();

            this->~Shader();
            return;
        }

        uniforms = new std::unordered_map<const char *, unsigned int>();
        attributes = new std::unordered_map<const char *, unsigned int>();
    }

    Shader::~Shader() {
        if(fragPtr != NULL) {
            glDeleteShader(fragPtr);
            fragPtr = NULL;
        }
        
        if(vertPtr != NULL) {
            glDeleteShader(vertPtr);
            vertPtr = NULL;
        }

        if(progPtr != NULL) {
            glDeleteProgram(progPtr);
            progPtr = NULL;
        }
    }

    void Shader::logProgram() {
        if(progPtr == NULL || !glIsProgram(progPtr)) return;

        int len = 0;
        int maxLen = len;

        glGetProgramiv(progPtr, GL_INFO_LOG_LENGTH, &maxLen);

        char *log = new char[maxLen];

        glGetProgramInfoLog(progPtr, maxLen, &len, log);
        if(len > 0) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", log);

        delete[] log;
    }

    void Shader::logShader(unsigned int shader) {
        if(shader == NULL || !glIsShader(shader)) return;

        int len = 0;
        int maxLen = len;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLen);

        char *log = new char[maxLen];

        glGetShaderInfoLog(shader, maxLen, &len, log);
        if(len > 0) SDL_Log("%s", log);

        delete[] log;
    }

    unsigned int Shader::createShader(int type, const char **source) {
        unsigned int shader = glCreateShader(type);
        if(shader == NULL) return NULL;

        glShaderSource(shader, 1, source, NULL);
        glCompileShader(shader);

        int compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        logShader(shader);
        if(compiled != GL_TRUE) {
            glDeleteShader(shader);
            return NULL;
        } else {
            return shader;
        }
    }

    unsigned int Shader::uniformLoc(const char *alias) {
        if(progPtr == NULL) throw std::exception("Program pointer not defined.");
        if(!uniforms->contains(alias)) {
            unsigned int loc = glGetUniformLocation(progPtr, alias);
            if(loc == -1) {
                std::string append(alias);
                throw std::exception(("Uniform not found: '" + append + "'.").c_str());
            } else {
                uniforms->emplace(alias, loc);
                return loc;
            }
        }

        return uniforms->at(alias);
    }

    unsigned int Shader::attributeLoc(const char *alias) {
        if(progPtr == NULL) throw std::exception("Program pointer not defined.");
        if(!attributes->contains(alias)) {
            unsigned int loc = glGetAttribLocation(progPtr, alias);
            if(loc == -1) {
                std::string append(alias);
                throw std::exception(("Attribute not found: '" + append + "'.").c_str());
            } else {
                attributes->emplace(alias, loc);
                return loc;
            }
        }

        return attributes->at(alias);
    }

    void Shader::bind() {
        if(progPtr == NULL) throw std::exception("Program pointer not defined.");
        glUseProgram(progPtr);
    }
}
