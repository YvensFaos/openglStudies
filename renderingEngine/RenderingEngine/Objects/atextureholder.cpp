#include "atextureholder.hpp"

#include "../Core/amodel.hpp"

ATextureHolder::ATextureHolder(const char *path, const std::string &directory, bool gamma, const GLint minFilter, const GLint magFilter, const GLint textureWrapS, const GLint textureWrapT) : 
    texture(AModel::TextureFromFile(path, directory, gamma, minFilter, magFilter, textureWrapS, textureWrapT))
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

GLuint ATextureHolder::getTextureID(void) const 
{
    return this->texture;
}