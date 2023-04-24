#include <SDL.h>
#include <GL/glew.h>
#include <string>
#include <stdexcept>

#include "shader.h"

namespace Fantasy {
    Shader::Shader(const char *vertSource, const char *fragSource): Shader(&vertSource, &fragSource) {}
    Shader::Shader(const char **vertSource, const char **fragSource) {
        vertPtr = createShader(GL_VERTEX_SHADER, vertSource);
        if(vertPtr == 0) throw std::runtime_error("Couldn't create vertex shader.");

        fragPtr = createShader(GL_FRAGMENT_SHADER, fragSource);
        if(fragPtr == 0) throw std::runtime_error("Couldn't create fragment shader.");

        progPtr = glCreateProgram();
        if(progPtr == 0) throw std::runtime_error("Couldn't create GL program.");

        glAttachShader(progPtr, vertPtr);
        glAttachShader(progPtr, fragPtr);
        glBindFragDataLocation(progPtr, 0, "fragColor");
        glLinkProgram(progPtr);

        int success;
        glGetProgramiv(progPtr, GL_LINK_STATUS, &success);
        if(success != GL_TRUE) throw std::runtime_error("Couldn't link GL program.");

        uniforms = new std::unordered_map<const char *, unsigned int>();
        attributes = new std::unordered_map<const char *, unsigned int>();
    }

    Shader::~Shader() {
        glDeleteShader(fragPtr);
        glDeleteShader(vertPtr);
        glDeleteProgram(progPtr);

        delete uniforms;
        delete attributes;
    }

    void Shader::logProgram() {
        if(!progPtr || !glIsProgram(progPtr)) return;

        int len = 0;
        int maxLen = len;

        glGetProgramiv(progPtr, GL_INFO_LOG_LENGTH, &maxLen);

        char *log = new char[maxLen];

        glGetProgramInfoLog(progPtr, maxLen, &len, log);
        if(len > 0) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", log);

        delete[] log;
    }

    void Shader::logShader(unsigned int shader) {
        if(!shader || !glIsShader(shader)) return;

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
        if(!shader) return 0;

        glShaderSource(shader, 1, source, 0);
        glCompileShader(shader);

        int compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        logShader(shader);
        if(compiled != GL_TRUE) {
            glDeleteShader(shader);
            return 0;
        } else {
            return shader;
        }
    }

    unsigned int Shader::uniformLoc(const char *alias) {
        if(!progPtr) throw std::runtime_error("Program pointer not defined.");
        if(!uniforms->count(alias)) {
            unsigned int loc = glGetUniformLocation(progPtr, alias);
            if(loc == -1) {
                std::string append(alias);
                throw std::runtime_error(("Uniform not found: '" + append + "'.").c_str());
            } else {
                uniforms->emplace(alias, loc);
                return loc;
            }
        }

        return uniforms->at(alias);
    }

    unsigned int Shader::attributeLoc(const char *alias) {
        if(!progPtr) throw std::runtime_error("Program pointer not defined.");
        if(!attributes->count(alias)) {
            unsigned int loc = glGetAttribLocation(progPtr, alias);
            if(loc == -1) {
                std::string append(alias);
                throw std::runtime_error(("Attribute not found: '" + append + "'.").c_str());
            } else {
                attributes->emplace(alias, loc);
                return loc;
            }
        }

        return attributes->at(alias);
    }

    void Shader::bind() {
        if(!progPtr) throw std::runtime_error("Program pointer not defined.");
        glUseProgram(progPtr);
    }
}
