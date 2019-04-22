#pragma once

#include <GL/glew.h>

#include <string>

class LuaHandler;

class ARenderQuad {
    private:
        GLuint vertexShader;
        GLuint fragmentShader;
        GLuint programme;
        GLuint quadVAO;
	    GLuint quadVBO;

        GLuint textureUniform;
        
        static std::string defaultVertexShader;
        static std::string defaultFragmentShader;
    public:
        ARenderQuad(void);
        ARenderQuad(std::string fragmentShaderText);
        ARenderQuad(std::string vertexShaderText, std::string fragmentShaderText);
        ~ARenderQuad(void);
        
        void render(GLuint texture) const;
    private:
        void initialize(std::string vertexShaderText, std::string fragmentShaderText);
};