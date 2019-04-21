#pragma once

#include <GL/glew.h>
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

        std::vector<std::string> faces;
    public:
        ASkybox(std::vector<std::string> faces);
        ~ASkybox(void);

        void render(void) const;
};