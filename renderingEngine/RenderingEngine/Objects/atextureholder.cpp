#include "atextureholder.hpp"

#include "../Core/amodel.hpp"

ATextureHolder::ATextureHolder(const GLfloat width, const GLfloat height, const GLint internalFormat, const GLenum format, const GLenum type, const GLint minFilter, const GLint magFilter) 
 : texture(this->generateTextId())
{
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

ATextureHolder::ATextureHolder(const char *path, const std::string &directory, bool gamma, const GLint minFilter, const GLint magFilter, const GLint textureWrapS, const GLint textureWrapT, const GLint internalFormat, const GLenum format) : 
    texture(AModel::TextureFromFile(path, directory, gamma, minFilter, magFilter, textureWrapS, textureWrapT))
{ }

ATextureHolder::ATextureHolder(const ATextureData& textureData, const GLint minFilter, const GLint magFilter, const GLint textureWrapS, const GLint textureWrapT, const GLint internalFormat, const GLenum format) : 
    texture(initializeFromTextureData(textureData, minFilter, magFilter, textureWrapS, textureWrapT, internalFormat, format)) 
{ }

ATextureHolder::ATextureHolder(const GLint texture) : texture(texture) 
{ }

ATextureHolder::~ATextureHolder(void) 
{
    glDeleteTextures(1, &texture);
}

void ATextureHolder::bindTexture(GLint textureUnit) const 
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, this->texture);
}

void ATextureHolder::unbindTexture(GLint textureUnit) const
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint ATextureHolder::generateTextId(void) {
    GLuint newTexture;
    glGenTextures(1, &newTexture);
    printf("Texture generated [%d].\n\n", newTexture);
    return newTexture;
}

GLuint ATextureHolder::initializeFromTextureData(const ATextureData& textureData, const GLint minFilter, const GLint magFilter, const GLint textureWrapS, const GLint textureWrapT, const GLint internalFormat, const GLenum format) 
{
    GLuint newTexture = generateTextId();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D, newTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, textureData.width, textureData.height, 0, format, GL_FLOAT, textureData.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapT);
    glBindTexture(GL_TEXTURE_2D, 0);

    return newTexture;
}