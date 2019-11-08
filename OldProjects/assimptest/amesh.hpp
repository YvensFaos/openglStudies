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
};  

class aiNode;
class aiScene;

class AMesh {
    public:
        std::vector<AVertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<ATexture> textures;

        AMesh(std::vector<AVertex> vertices, std::vector<unsigned int> indices, std::vector<ATexture> textures);
        void Draw(GLuint shader);
    private:
        unsigned int VAO, VBO, EBO;
        void setupMesh();
};