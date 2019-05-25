#include "amesh.hpp"

#include <GL/glew.h>

AMesh::AMesh(std::vector<AVertex> vertices, std::vector<unsigned int> indices, std::vector<ATexture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->setupMesh();
}

void AMesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
  
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(AVertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
                 &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AVertex), (void*)0);
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AVertex), (void*)offsetof(AVertex, Normal));
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AVertex), (void*)offsetof(AVertex, TexCoords));
    glEnableVertexAttribArray(3);	
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(AVertex), (void*)offsetof(AVertex, Tangent));
    glBindVertexArray(0);
}

void AMesh::bindTextures(GLuint shader) const
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;

    unsigned int number;

    for(size_t i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);

        std::string name = textures[i].type;

        if(name == "texture_diffuse") 
        {
            number = diffuseNr++;
        }
        else if(name == "texture_specular") 
        {
            number = specularNr++;
        }
        else if(name == "texture_normal") 
        {
            number = normalNr++;
        }

        std::string textIdentifier(name + std::to_string(number));
        glUniform1i(glGetUniformLocation(shader, textIdentifier.c_str()), i); 
    }
}

void AMesh::draw(GLuint shader) const 
{
    bindTextures(shader);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}  