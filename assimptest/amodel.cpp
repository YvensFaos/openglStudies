#include <GL/glew.h>
#include "amodel.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <vector>
#include <stack>

AModel::AModel(std::string path) 
{ 
    loadModel(path); 
}

void AModel::loadModel(std::string path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals); 

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        printf("Something went wrong. Unable to load file: %s\n", path.c_str());
        return;
    }
    std::string directory = path.substr(0, path.find_last_of('/'));
    glm::mat4 parentMat4 = this->aiMatrix4x4ToGlm(&scene->mRootNode->mTransformation);
    processNode(scene->mRootNode, scene, parentMat4);
}

void AModel::processNode(aiNode *node, const aiScene *scene, const glm::mat4 parentMat4)
{
    glm::mat4 currentTransform = this->aiMatrix4x4ToGlm(&node->mTransformation);
    glm::mat4 resulting = parentMat4 * currentTransform;

    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, resulting));
    }
    for(unsigned int j = 0; j < node->mNumChildren; j++)
    {
        this->processNode(node->mChildren[j], scene, resulting);
    }
} 

void AModel::Draw(unsigned int shader)
{
    for(unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shader);
    }
}  

glm::mat4 AModel::aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    glm::mat4 to;
    to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
    to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
    to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
    to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;
    return to;
}

AMesh AModel::processMesh(aiMesh *mesh, const aiScene *scene, glm::mat4 transformation)
{
    std::vector<AVertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<ATexture> textures;

    glm::vec4 vector; 
    glm::vec2 vec;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        AVertex vertex;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z; 
        vector.w = 1.0f;
        glm::vec4 tv = transformation * vector;
        vertex.Position.x = tv.x;
        vertex.Position.y = tv.y;
        vertex.Position.z = tv.z;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal.x = vector.x; 
        vertex.Normal.y = vector.y; 
        vertex.Normal.z = vector.z; 

        if(mesh->mTextureCoords[0])
        {
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);  
        }
        
        vertices.push_back(vertex);
    }

    aiFace face;
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    return AMesh(vertices, indices, textures);
}  