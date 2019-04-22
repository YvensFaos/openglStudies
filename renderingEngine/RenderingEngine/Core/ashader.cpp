#include "ashader.hpp"

#include <vector>

GLuint AShader::generateShader(std::string shaderText, GLuint shaderType) 
{
	GLuint shader = glCreateShader(shaderType);
	const char *s_str = shaderText.c_str();
    glShaderSource(shader, 1, &s_str, NULL);
    glCompileShader(shader);
	GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(success == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, NULL, &errorLog[0]);
        std::string errorMessage(begin(errorLog), end(errorLog));
        glDeleteShader(shader);

		printf("Shader Error: %s\n", errorMessage.c_str());
    }
    else {
        printf("Shader compiled [%d]: %s\n", shader, shaderText.c_str());
    }
	return shader;
}

GLuint AShader::generateProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint shaderProgramme = glCreateProgram();
    glAttachShader(shaderProgramme, vertexShader);
    glAttachShader(shaderProgramme, fragmentShader);
    glLinkProgram(shaderProgramme);

    return shaderProgramme;
}