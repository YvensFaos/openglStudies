#include <GL/glew.h>
#include "amodel.hpp"

#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

AModel::AModel(std::string path) : modelMatrix(glm::mat4(1.0))
{ 
    loadModel(path); 
}

void AModel::loadModel(std::string path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace); 

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    printf("Loading object: %s\r\nDirectory: %s\r\n", path.c_str(), directory.c_str());
    glm::mat4 parentMat4 = AModel::aiMatrix4x4ToGlm(&scene->mRootNode->mTransformation);
    processNode(scene->mRootNode, scene, parentMat4);
}

void AModel::processNode(aiNode *node, const aiScene *scene, const glm::mat4 parentMat4)
{
    glm::mat4 currentTransform = AModel::aiMatrix4x4ToGlm(&node->mTransformation);
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

void AModel::draw(GLuint programme, GLenum mode) const
{
    for(unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].draw(programme, mode);
    }
}  

void AModel::renderModels(GLuint modelMatrixUniform, GLuint programme, GLenum mode) const
{
    auto pointer = this->getMeshes();
    auto modelMatrix = this->getModelMatrix();

    glUniformMatrix4fv (modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    for (size_t i = 0; i < pointer.size(); i++)
    {
        pointer[i].draw(programme, mode);
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

    glm::vec4 vector4; 
    glm::vec2 vec;
    glm::vec4 tv;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        AVertex vertex;
        vector4.x = mesh->mVertices[i].x;
        vector4.y = mesh->mVertices[i].y;
        vector4.z = mesh->mVertices[i].z; 
        vector4.w = 1.0f;
        
        tv = transformation * vector4;
        vertex.Position.x = tv.x;
        vertex.Position.y = tv.y;
        vertex.Position.z = tv.z;

        vector4.x = mesh->mNormals[i].x;
        vector4.y = mesh->mNormals[i].y;
        vector4.z = mesh->mNormals[i].z;
        vertex.Normal.x = vector4.x; 
        vertex.Normal.y = vector4.y; 
        vertex.Normal.z = vector4.z; 

        if(mesh->mTangents)
        {
            vector4.x = mesh->mTangents[i].x;
            vector4.y = mesh->mTangents[i].y;
            vector4.z = mesh->mTangents[i].z;
        }

        vertex.Tangent.x = vector4.x;
        vertex.Tangent.y = vector4.y;
        vertex.Tangent.z = vector4.z;

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

    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<ATexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<ATexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<ATexture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }  
    return AMesh(vertices, indices, textures);
}

std::vector<ATexture> AModel::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<ATexture> textures;
    for(size_t i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        ATexture texture;
        texture.id = TextureFromFile(str.C_Str(), directory);
        texture.type = typeName;
        texture.path = std::string(str.C_Str());
        textures.push_back(texture);
    }
    return textures;
}

unsigned int AModel::TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    if(!directory.empty()) {
        filename = directory + '/' + filename;
    }
    unsigned int textureID;

    glGenTextures(1, &textureID);
    printf("Loading texture [%d]: %s ... ", textureID, filename.c_str());

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
        {
            format = GL_RED;
        }
        else if (nrComponents == 3)
        {
            format = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            format = GL_RGBA;
        }

        printf("Loaded with %d x %d [Components: %d]!\r\n", width, height, nrComponents);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        // glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    }
    else
    {
        printf("Texture failed to load at path: %s\n", path);
        stbi_image_free(data);
    }

    return textureID;
}

const std::vector<AMesh>& AModel::getMeshes(void) const
{
    return this->meshes;
}

void AModel::translate(glm::vec3 translateTo) 
{
    this->modelMatrix = glm::translate(this->modelMatrix, translateTo);
}

void AModel::rotate(glm::vec3 rotateTo) 
{
    this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(rotateTo.x), glm::vec3(1, 0, 0));
    this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(rotateTo.y), glm::vec3(0, 1, 0));
    this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(rotateTo.z), glm::vec3(0, 0, 1));
}

void AModel::scale(glm::vec3 scaleTo) 
{
    this->modelMatrix = glm::scale(this->modelMatrix, scaleTo);
}

glm::mat4 AModel::getModelMatrix(void) const 
{
    return this->modelMatrix;
}

glm::vec4 AModel::getPosition(void) const 
{
    return this->modelMatrix[3];
}

void AModel::setPosition(glm::vec3 position) {
    this->modelMatrix[3] = glm::vec4(position, 1.0f);
}

ABoundingBox AModel::getBoundingBox(void) const {
    glm::vec3 min(+999999.0f, +999999.0f, +999999.0f);
    glm::vec3 max(-999999.0f, -999999.0f, -999999.0f);

    glm::vec3 position;
    ABoundingBox abb;
    for(auto ameshIterator = this->meshes.begin(); ameshIterator != this->meshes.end(); ++ameshIterator)
    {
        abb = (*ameshIterator).getBoundingBox();
        min.x = std::min(min.x, abb.getMin().x);
        min.y = std::min(min.y, abb.getMin().y);
        min.z = std::min(min.z, abb.getMin().z);
        max.x = std::max(max.x, abb.getMax().x);
        max.y = std::max(max.y, abb.getMax().y);
        max.z = std::max(max.z, abb.getMax().z);
    }

    return ABoundingBox(min, max);
}

void AModel::renderModelsInList(std::vector<AModel*>* list, GLuint modelMatrixUniform, GLuint programme, GLenum mode)
{
    std::vector<AModel*>::iterator amodelIterator;
    for(amodelIterator = list->begin(); amodelIterator != list->end(); ++amodelIterator)
    {
        const std::vector<AMesh>& pointer = (*amodelIterator)->getMeshes();
        auto modelMatrix = (*amodelIterator)->getModelMatrix();
        for(unsigned int i = 0; i < pointer.size(); i++)
        {
            glUniformMatrix4fv (modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            pointer[i].draw(programme, mode);
        }
    }
}

const AMesh& AModel::getMeshAt(int index) const 
{
    return this->meshes[index];
}