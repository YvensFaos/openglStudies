#pragma once

#include <GL/glew.h>
#include <string>

class AShader {
    public:
        static GLuint generateShader(std::string shaderText, GLuint shaderType);
        static GLuint generateProgram(GLuint vertexShader, GLuint fragmentShader);
};