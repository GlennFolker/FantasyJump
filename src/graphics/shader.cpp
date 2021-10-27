#include <SDL.h>
#include <gl/glew.h>

#include "shader.h"

namespace Fantasy {
    Shader::Shader(const char *vertSource, const char *fragSource): Shader((char**)&vertSource, (char**)&fragSource) {}

    Shader::Shader(char **vertSource, char **fragSource) {
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
        glLinkProgram(progPtr);

        int success;
        glGetProgramiv(progPtr, GL_LINK_STATUS, &success);
        if(success != GL_TRUE) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't link shader program.");
            logProgram();

            this->~Shader();
            return;
        }
    }

    Shader::~Shader() {
        if(fragPtr != NULL) glDeleteShader(fragPtr);
        if(vertPtr != NULL) glDeleteShader(vertPtr);
        if(progPtr != NULL) glDeleteProgram(progPtr);
    }

    void Shader::logProgram() {
        if(progPtr == NULL || !glIsProgram(progPtr)) return;

        int len = 0;
        int maxLen = len;

        glGetProgramiv(progPtr, GL_INFO_LOG_LENGTH, &maxLen);

        char *log = new char[maxLen];

        glGetProgramInfoLog(progPtr, maxLen, &len, log);
        if(len > 0) SDL_Log("%s\n", log);

        delete[] log;
    }

    void Shader::logShader(GLuint shader) {
        if(shader == NULL || !glIsShader(shader)) return;

        int len = 0;
        int maxLen = len;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLen);

        char *log = new char[maxLen];

        glGetShaderInfoLog(shader, maxLen, &len, log);
        if(len > 0) SDL_Log("%s", log);

        delete[] log;
    }

    GLuint Shader::createShader(int type, char **source) {
        GLuint shader = glCreateShader(type);
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
}
