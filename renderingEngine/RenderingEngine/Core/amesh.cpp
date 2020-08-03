#include "amesh.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../Utils/amacrohelper.hpp"

AMesh::AMesh(std::vector<AVertex> vertices, std::vector<unsigned int> indices, std::vector<ATexture> textures) 
    : vertices(vertices), indices(indices), textures(textures)
{
    this->setupMesh();
}

AMesh::AMesh(const AMesh& anotherMesh) : VAO(anotherMesh.getVAO()), VBO(anotherMesh.getVBO()), EBO(anotherMesh.getEBO()),
    vertices(anotherMesh.getVertices()), indices(anotherMesh.getIndices()), textures(anotherMesh.getTextures())
{ }

AMesh::AMesh(const AMesh& anotherMesh, bool generateOwnObjects) 
    : vertices(anotherMesh.vertices), indices(anotherMesh.indices), textures(anotherMesh.textures) 
{
    this->setupMesh();
}

AMesh& AMesh::operator=(const AMesh& anotherMesh) 
{
    if(this != &anotherMesh) {
        this->VAO = anotherMesh.getVAO();
        this->VBO = anotherMesh.getVBO();
        this->EBO = anotherMesh.getEBO();
        this->vertices = anotherMesh.getVertices();
        this->indices = anotherMesh.getIndices();
        this->textures = anotherMesh.getTextures();
    }

    return *this;
}

void AMesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    printf("VAO Generated = %d\n", VAO);
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

void AMesh::setVAO(GLuint VAO) {
    this->VAO = VAO;
}

void AMesh::setVBO(GLuint VBO) {
    this->VBO = VBO;
} 

void AMesh::setEBO(GLuint EBO) {
    this->EBO = EBO;
}

void AMesh::draw(GLuint shader, GLenum mode, bool renderWithTextures) const 
{
    if(renderWithTextures) {
        this->bindTextures(shader);
    }
    glBindVertexArray(VAO);
    glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, 0);
}

void AMesh::drawPatches(GLuint shader, bool renderWithTextures) const 
{
    if(renderWithTextures) {
        this->bindTextures(shader);
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_PATCHES, indices.size(), GL_UNSIGNED_INT, 0);
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

AVertex AMesh::generateVertex(glm::vec3 position, glm::vec3 normal, glm::vec3 tangent, glm::vec2 texCoords) {
    AVertex generatedVertex;
    generatedVertex.Position = position;
    generatedVertex.Normal = normal;
    generatedVertex.Tangent = tangent;
    generatedVertex.TexCoords = texCoords;

    return generatedVertex;
}

///AInstanceMesh

AInstanceMesh::AInstanceMesh(std::vector<AVertex> vertices, std::vector<GLuint> indices, std::vector<ATexture> textures, std::vector<glm::mat4> instanceData) :
 AMesh(vertices, indices, textures), instanceData(instanceData) {
    this->setupInstanceBO();
}

AInstanceMesh::AInstanceMesh(const AInstanceMesh& anotherInstanceMesh) : 
 AMesh(anotherInstanceMesh), instanceData(anotherInstanceMesh.getInstanceData()), IBO(anotherInstanceMesh.getIBO()) 
{ }

AInstanceMesh::AInstanceMesh(const AMesh& meshToCopyFrom, std::vector<glm::mat4> instanceData) : 
 AMesh(meshToCopyFrom, true), instanceData(instanceData) {
    this->setupInstanceBO();
 }

void AInstanceMesh::draw(GLuint shader, GLenum mode, bool renderWithTextures) const {
    if(renderWithTextures) {
        this->bindTextures(shader);
    }

    glBindVertexArray(this->getVAO());
    glDrawElementsInstanced(mode, indices.size(), GL_UNSIGNED_INT, 0, this->getInstanceCount());
}

const std::vector<glm::mat4>& AInstanceMesh::getInstanceData(void) const {
    return this->instanceData;
}

const GLuint AInstanceMesh::getInstanceCount(void) const {
    return this->instanceData.size();
}

const GLuint AInstanceMesh::getIBO(void) const {
    return this->IBO;
}

void AInstanceMesh::setupInstanceBO(void) {
    const auto sizeOfMat4 = sizeof(glm::mat4);

    glBindVertexArray(this->getVAO());

    glGenBuffers(1, &this->IBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->IBO);
    glBufferData(GL_ARRAY_BUFFER, this->getInstanceCount() * sizeof(glm::mat4), &instanceData[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (0 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (1 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    
    glBindVertexArray(0);
}

glm::mat4 AInstanceMesh::fromValuesToInstanceMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    glm::mat4 instanceMatrix = glm::mat4(1.0f);
    instanceMatrix = glm::translate(instanceMatrix, position);
    instanceMatrix = glm::scale(instanceMatrix, scale);
    instanceMatrix = glm::rotate(instanceMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    instanceMatrix = glm::rotate(instanceMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    instanceMatrix = glm::rotate(instanceMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    return instanceMatrix;
}

AInstanceMesh& AInstanceMesh::operator=(const AInstanceMesh& anotherMesh) 
{
    if(this != &anotherMesh) {
        this->setVAO(anotherMesh.getVAO());
        this->setVBO(anotherMesh.getVBO());
        this->setEBO(anotherMesh.getEBO());
        this->IBO = anotherMesh.getIBO();
        this->vertices = anotherMesh.getVertices();
        this->indices = anotherMesh.getIndices();
        this->textures = anotherMesh.getTextures();
        this->instanceData = anotherMesh.getInstanceData();
    }

    return *this;
}