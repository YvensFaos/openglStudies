#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct AVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
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
        void bindTextures(GLuint shader);
        void draw(void) const;
    private:
        GLuint VAO, VBO, EBO;
        void setupMesh();
};