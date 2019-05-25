#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

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

        AMesh(std::vector<AVertex> vertices, std::vector<GLuint> indices, std::vector<ATexture> textures);
        void draw(GLuint shader) const;
    private:
        GLuint VAO, VBO, EBO;
        void setupMesh();
        void bindTextures(GLuint shader) const;
};