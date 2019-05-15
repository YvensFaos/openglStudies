#include "askybox.hpp"

#include <stb_image.h>
#include <glm/gtc/type_ptr.hpp>

#include "../Core/ashader.hpp"

float vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

std::string ASkybox::defaultVertexShader = 
    "#version 400\n"
    "   layout (location = 0) in vec3 aPos;\n"
    "   uniform mat4 vpMatrix;\n"
    "   out vec3 texCoord;\n"
    "   void main()\n"
    "   {\n"
    "       texCoord = aPos;\n"
    "       vec4 pos = vpMatrix * vec4(aPos, 1.0);\n"
    "       gl_Position = pos.xyww;\n"
    "   }\n";

std::string ASkybox::defaultFragmentShader = 
    "#version 400\n"
    "   in vec3 texCoord;\n"
    "   uniform samplerCube skybox;\n"
    "   out vec4 frag_colour;\n"
    "   void main()\n"
    "   {   \n"
    "       vec4 color = texture(skybox, texCoord);\n"
    "       frag_colour = color;\n"
    "   }\n";

ASkybox::ASkybox(std::vector<std::string> faces) : faces(faces) 
{
    vertexShader = AShader::generateShader(defaultVertexShader, GL_VERTEX_SHADER);
	fragmentShader = AShader::generateShader(defaultFragmentShader, GL_FRAGMENT_SHADER);
    programme = AShader::generateProgram(vertexShader, fragmentShader);
    skyboxUniform = glGetUniformLocation(programme, "skybox");
    viewProjectionUniform = glGetUniformLocation(programme, "vpMatrix");

	glGenTextures(1, &cubemapTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
	unsigned char *data;

	for(GLuint i = 0; i < faces.size(); i++)
	{
		data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
}

ASkybox::~ASkybox(void)
{

}

void ASkybox::render(glm::mat4 viewProjectionMatrix) const 
{
    glUseProgram(programme);
    glUniformMatrix4fv (viewProjectionUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
    glUniform1i(skyboxUniform, 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); 
}