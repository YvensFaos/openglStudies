#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>

class LuaHandler;

class ARenderQuad {
    protected:
        GLuint vertexShader;
        GLuint fragmentShader;
        GLuint programme;

        GLuint textureUniform;
        GLuint quadVAO;
	    GLuint quadVBO;
    protected:
        static std::string defaultVertexShader;
        static std::string defaultFragmentShader;
    public:
        ARenderQuad(void);
        ARenderQuad(std::string fragmentShaderText);
        ARenderQuad(std::string vertexShaderText, std::string fragmentShaderText);
        virtual ~ARenderQuad(void);
                
        void render(GLuint texture, bool setupProgramme = true) const;
        void renderCubemap(GLuint cubemapTexture) const;
        void render(std::vector<GLuint> textures) const;
        GLuint getProgramme(void) const;
    protected:
        void initialize(std::string vertexShaderText, std::string fragmentShaderText);
};