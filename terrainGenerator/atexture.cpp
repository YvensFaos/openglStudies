#include "atexture.hpp"

#include <stdio.h>

ATextureHolder::ATextureHolder(const GLuint textureWidth, const GLuint textureHeight) : textureWidth(textureWidth), textureHeight(textureHeight) {
    this->initializeTexture();
}

ATextureHolder::ATextureHolder(const ATextureData& data) : textureWidth(data.width), textureHeight(data.height) {
    this->initializeTexture();
    this->setTextureData(data);
}

ATextureHolder::~ATextureHolder(void) {
    glDeleteTextures(1, &this->textureID);
}

const GLuint ATextureHolder::getTextureID(void) const {
    return this->textureID;
}

void ATextureHolder::initializeTexture(void) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &this->textureID);
    printf("Texture generated [%d].\n\n", this->textureID);
}

void ATextureHolder::setTextureData(const ATextureData& textureData) {
    glBindTexture(GL_TEXTURE_2D, this->textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, textureData.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ATextureHolder::bindTexture(GLuint activateTextureIndex) const {
    glActiveTexture(GL_TEXTURE0 + activateTextureIndex);
    glBindTexture(GL_TEXTURE_2D, this->textureID);
}

void ATextureHolder::unbindTexture(GLuint activateTextureIndex) const {
    glActiveTexture(GL_TEXTURE0 + activateTextureIndex);
    glBindTexture(GL_TEXTURE_2D, 0);
}
