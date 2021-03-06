#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "aboundingbox.hpp"

struct AVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    glm::vec2 TexCoords;
};

struct ATexture {
    GLuint id;
    std::string type;
    std::string path;
};

class AMesh {
    public:
        std::vector<AVertex> vertices;
        std::vector<GLuint> indices;
        std::vector<ATexture> textures;
    private:
        GLuint VAO;
        GLuint VBO; 
        GLuint EBO;
    public:
        AMesh(std::vector<AVertex> vertices, std::vector<GLuint> indices, std::vector<ATexture> textures);
        AMesh(const AMesh& anotherMesh);

        void virtual draw(GLuint shader, GLenum mode, bool renderWithTextures) const;
        void drawPatches(GLuint shader, bool renderWithTextures) const;
        ABoundingBox getBoundingBox(void) const;

        const std::vector<AVertex>& getVertices(void) const;
        const std::vector<GLuint>& getIndices(void) const;
        const std::vector<ATexture>& getTextures(void) const;

        const GLuint getVAO(void) const;
        const GLuint getVBO(void) const;
        const GLuint getEBO(void) const;

        AMesh& operator=(const AMesh& anotherMesh);
    private:
        void setupMesh();
    protected:
        AMesh(const AMesh& anotherMesh, bool generateOwnObjects);
        void bindTextures(GLuint shader) const;

        void setVAO(GLuint VAO);
        void setVBO(GLuint VBO); 
        void setEBO(GLuint EBO);
    public:
        static AVertex generateVertex(glm::vec3 position, glm::vec3 normal = glm::vec3(0.0f,0.0f,0.0f), glm::vec3 tangent = glm::vec3(0.0f,0.0f,0.0f), glm::vec2 texCoords = glm::vec2(0.0f,0.0f));
};

class AInstanceMesh : public AMesh {
    public:
        std::vector<glm::mat4> instanceData;
    private:
        GLuint IBO;
    public:
        AInstanceMesh(std::vector<AVertex> vertices, std::vector<GLuint> indices, std::vector<ATexture> textures, std::vector<glm::mat4> instanceData);
        AInstanceMesh(const AInstanceMesh& anotherInstanceMesh);
        AInstanceMesh(const AMesh& meshToCopyFrom, std::vector<glm::mat4> instanceData);

        const std::vector<glm::mat4>& getInstanceData(void) const;
        const GLuint getInstanceCount(void) const;
        const GLuint getIBO(void) const;

        void draw(GLuint shader, GLenum mode, bool renderWithTextures) const override;

        AInstanceMesh& operator=(const AInstanceMesh& anotherMesh);
    private:
        void setupInstanceBO(void);

    public:
        static glm::mat4 fromValuesToInstanceMatrix(glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f,0.0f,0.0f), glm::vec3 scale = glm::vec3(1.0f,1.0f,1.0f));
};