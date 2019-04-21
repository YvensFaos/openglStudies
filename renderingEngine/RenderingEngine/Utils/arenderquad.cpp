#include "arenderquad.hpp"

#include <GL/glew.h>

#include "../Core/ashader.hpp"
#include "luahandler.hpp"

float quadVertices[] = {
    -1.0f,  1.0f,  0.0f,  0.0f, 
     1.0f, -1.0f, -1.0f,  0.0f, 
     0.0f,  0.0f,  1.0f,  1.0f, 
     0.0f,  1.0f,  1.0f,  1.0f, 
    -1.0f,  0.0f,  1.0f,  0.0f,
};

ARenderQuad::ARenderQuad(std::string vertexShaderText, std::string fragmentShaderText) 
{
    vertexShader = AShader::generateShader(vertexShaderText, GL_VERTEX_SHADER);
	fragmentShader = AShader::generateShader(fragmentShaderText, GL_FRAGMENT_SHADER);
    programme = AShader::generateProgram(vertexShader, fragmentShader);
    textureUniform = glGetUniformLocation(programme, "textureUniform");
    
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

ARenderQuad::~ARenderQuad(void)
{

}

void ARenderQuad::render(GLuint texture) const 
{
    glUseProgram(programme);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(textureUniform, 0);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}