#include <SDL.h>
#include <gl/glew.h>

#include "shader.h"

namespace Fantasy {
    Shader::Shader(char **vertSource, char **fragSource) {
        progPtr = glCreateProgram();
        if(progPtr == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't create GL program.");
            this->~Shader();
        }

        vertPtr = glCreateShader(GL_VERTEX_SHADER);
        if(vertPtr == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't create vertex shader.");
            this->~Shader();
        } else {
            glAttachShader(progPtr, vertPtr);
        }

        fragPtr = glCreateShader(GL_FRAGMENT_SHADER);
        if(fragPtr == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't create fragment shader.");
            this->~Shader();
        } else {
            glAttachShader(progPtr, fragPtr);
        }

        glLinkProgram(progPtr);

        int success;
        glGetProgramiv(progPtr, GL_LINK_STATUS, &success);
        if(success != GL_TRUE) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't link shader program.");
            this->~Shader();
        }
    }

    Shader::~Shader() {
        if(fragPtr != NULL) glDeleteShader(fragPtr);
        if(vertPtr != NULL) glDeleteShader(vertPtr);
        if(progPtr != NULL) glDeleteProgram(progPtr);
    }

    GLuint Shader::createShader(int type, char **source) {
        GLuint shader = glCreateShader(type);
        if(shader == NULL) return NULL;

        glShaderSource(shader, 1, source, NULL);
        glCompileShader(shader);

        int compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if(compiled != GL_TRUE) {
            glDeleteShader(shader);
            return NULL;
        } else {
            return shader;
        }
    }
}
