#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class ASkybox{
    private:
    	GLuint cubemapTextureID;
        GLint width;
        GLint height;
        GLint channels;
        GLuint VAO;
        GLuint VBO;

        GLuint vertexShader;
        GLuint fragmentShader;
        GLuint programme;
        GLuint viewProjectionUniform;
        GLuint skyboxUniform;

        std::vector<std::string> faces;

    private:
        static std::string defaultVertexShader;
        static std::string defaultFragmentShader;
    public:
        ASkybox(std::vector<std::string> faces);
        ~ASkybox(void);

        void render(glm::mat4 viewProjectionMatrix) const;
};