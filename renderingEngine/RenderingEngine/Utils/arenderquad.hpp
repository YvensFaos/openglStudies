#pragma once

#include <GL/glew.h>

#include <string>

class LuaHandler;

class ARenderQuad {
    public:

    private:
        GLuint vertexShader;
        GLuint fragmentShader;
        GLuint programme;
        GLuint quadVAO;
	    GLuint quadVBO;

        GLuint textureUniform;
    public:
        ARenderQuad(std::string vertexShaderText, std::string fragmentShaderText);
        ~ARenderQuad(void);
        
        void render(GLuint texture) const;
};