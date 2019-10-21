#include "amesh.hpp"

#include <GL/glew.h>

AMesh::AMesh(std::vector<AVertex> vertices, std::vector<unsigned int> indices, std::vector<ATexture> textures) 
    : vertices(vertices), indices(indices), textures(textures)
{
    this->setupMesh();
}

AMesh::AMesh(const AMesh& anotherMesh) : VAO(anotherMesh.getVAO()), VBO(anotherMesh.getVBO()), EBO(anotherMesh.getEBO()),
    vertices(anotherMesh.getVertices()), indices(anotherMesh.getIndices()), textures(anotherMesh.getTextures())
{ }

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

void AMesh::draw(GLuint shader, GLenum mode) const 
{
    bindTextures(shader);
    glBindVertexArray(VAO);
    glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, 0);
}  

ABoundingBox AMesh::getBoundingBox(void) const 
{
    glm::vec3 min(+999999.0f, +999999.0f, +999999.0f);
    glm::vec3 max(-999999.0f, -999999.0f, -999999.0f);

    glm::vec3 position;
    for(size_t i = 0; i < this->vertices.size(); i++)
    {
        position = this->vertices[i].Position;
        min.x = std::min(min.x, position.x);
        min.y = std::min(min.y, position.y);
        min.z = std::min(min.z, position.z);

        max.x = std::max(max.x, position.x);
        max.y = std::max(max.y, position.y);
        max.z = std::max(max.z, position.z);
    }

    return ABoundingBox(min, max);
}

const std::vector<AVertex>& AMesh::getVertices(void) const {
    return this->vertices;
}

const std::vector<GLuint>& AMesh::getIndices(void) const {
    return this->indices;
}

const std::vector<ATexture>& AMesh::getTextures(void) const {
    return this->textures;
}

const GLuint AMesh::getVAO(void) const {
    return this->VAO;
}
const GLuint AMesh::getVBO(void) const {
    return this->VBO;
}
const GLuint AMesh::getEBO(void) const {
    return this->EBO;
}