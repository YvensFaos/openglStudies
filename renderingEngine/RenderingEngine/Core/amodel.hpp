#pragma once

#include <string>
#include <vector>

#include "amesh.hpp"
#include "aboundingbox.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

class AModel 
{
    private: 
        glm::mat4 modelMatrix;
        std::vector<AMesh> meshes;
        std::string directory;

    public:
        AModel(std::string path);
        AModel(const AModel& copyFrom);

        void draw(GLuint programme, GLenum mode = GL_TRIANGLES, bool renderWithTextures = true) const;
        void renderModels(GLuint modelMatrixUniform, GLuint programme, GLenum mode = GL_TRIANGLES, bool renderWithTextures = true) const;
        const std::vector<AMesh>& getMeshes(void) const;
        const AMesh& getMeshAt(int index) const;

        void translate(glm::vec3 translateTo);
        void rotate(glm::vec3 rotateTo);
        void scale(glm::vec3 scaleTo);
        
        glm::mat4 getModelMatrix(void) const;
        glm::vec4 getPosition(void) const;
        std::string getDirectory(void) const;
        
        void setPosition(glm::vec3 position);
        ABoundingBox getBoundingBox(void) const;

        AModel& operator=(const AModel& copyFrom);
    private:
        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene, const glm::mat4 parentMat4);
        AMesh processMesh(aiMesh *mesh, const aiScene *scene, const glm::mat4 transformation);
        std::vector<ATexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

    public:
        static unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false, const GLint minFilter = GL_LINEAR, const GLint magFilter = GL_LINEAR, const GLint textureWrapS = GL_CLAMP_TO_BORDER, const GLint textureWrapT = GL_CLAMP_TO_BORDER);
        static void renderModelsInList(std::vector<AModel>& list, GLuint modelMatrixUniform, GLuint programme, GLenum mode = GL_TRIANGLES, bool renderWithTextures = true);
        static glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from);
};