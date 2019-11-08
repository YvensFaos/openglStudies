#pragma once

#include <string>
#include <vector>

#include "amesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
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
        std::vector<ATexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
        unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
};