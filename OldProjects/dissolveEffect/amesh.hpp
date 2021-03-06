#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

typedef unsigned int GLuint;

struct AVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct ATexture {
    unsigned int id;
    std::string type;
    std::string path;
};  

class AMesh {
    public:
        std::vector<AVertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<ATexture> textures;

        AMesh(std::vector<AVertex> vertices, std::vector<unsigned int> indices, std::vector<ATexture> textures);
        void BindTextures(GLuint shader);
        void Draw(GLuint shader);
    private:
        unsigned int VAO, VBO, EBO;
        void setupMesh();
};