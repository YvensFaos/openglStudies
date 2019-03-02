#pragma once

#include <string>

#include "amesh.hpp"

class aiNode;
class aiScene;
class aiMesh;

#include <assimp/matrix4x4.h>
#include <glm/glm.hpp>

class AModel 
{
    public:
        AModel(std::string path);
        void Draw(unsigned int shader);
        
    private:
        std::vector<AMesh> meshes;
        std::string directory;

    private:
        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene, const glm::mat4 parentMat4);
        AMesh processMesh(aiMesh *mesh, const aiScene *scene, const glm::mat4 transformation);
        glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from);
};