#include "arenderquad.hpp"

#include <GL/glew.h>

#include "../Core/ashader.hpp"

float quadVertices[] = {
    -1.0f,  1.0f,  0.0f,  0.0f, 
     1.0f, -1.0f, -1.0f,  0.0f, 
     0.0f,  0.0f,  1.0f,  1.0f, 
     0.0f,  1.0f,  1.0f,  1.0f, 
    -1.0f,  0.0f,  1.0f,  0.0f,
};

std::string ARenderQuad::defaultVertexShader = 
    "#version 400\n"
    "    layout (location = 0) in vec3 vertex;\n"
    "    layout (location = 1) in vec2 uv;\n"
    "    out vec2 vuv;\n"
    "    void main()\n"
    "    {\n"
    "        vuv = uv;\n"
    "        gl_Position = vec4(vertex, 1.0);\n"
    "    }\n";

std::string ARenderQuad::defaultFragmentShader = 
    "#version 400\n"
    "   in vec2 vuv;\n"
    "   uniform sampler2D textureUniform;\n"
    "   out vec4 frag_colour;\n"
    "   void main()\n"
    "   {             "
    "       vec4 value = texture(textureUniform, vuv);"
    "       frag_colour = vec4(vec3(value), 1.0);"
    "   }";

ARenderQuad::ARenderQuad(void) 
{
    initialize(ARenderQuad::defaultVertexShader, ARenderQuad::defaultFragmentShader);
}

ARenderQuad::ARenderQuad(std::string fragmentShaderText) 
{
    initialize(ARenderQuad::defaultVertexShader, fragmentShaderText);
}

ARenderQuad::ARenderQuad(std::string vertexShaderText, std::string fragmentShaderText) 
{
    initialize(vertexShaderText, fragmentShaderText);
}

ARenderQuad::~ARenderQuad(void)
{
    glDeleteProgram(programme);
}

void ARenderQuad::render(GLuint texture, bool setupProgramme) const 
{
    if(setupProgramme)
    {
        glUseProgram(programme);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(textureUniform, 0);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void ARenderQuad::renderCubemap(GLuint cubemapTexture) const 
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glUniform1i(textureUniform, 0);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);
}

void ARenderQuad::render(std::vector<GLuint> textures) const
{
    for(size_t i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        std::string textureName("textureUniform" + std::to_string(i));
        glUniform1i(glGetUniformLocation(programme, textureName.c_str()), i);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void ARenderQuad::initialize(std::string vertexShaderText, std::string fragmentShaderText)
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

GLuint ARenderQuad::getProgramme(void) const
{
    return this->programme;
}