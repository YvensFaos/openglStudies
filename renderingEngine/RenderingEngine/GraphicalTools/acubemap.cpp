#include "acubemap.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "aframebuffer.hpp"

ACubeMap::ACubeMap(AFramebuffer& aframebuffer, GLuint width, GLuint height, GLuint internalFormat, GLuint format, GLuint type) : aframebuffer(aframebuffer) {
    glGenTextures(1, &cubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
    
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, type, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  
}

ACubeMap::~ACubeMap(void) {
    //TODO implement
}

void ACubeMap::setFramebufferParameters(GLuint attachments, bool turnOffRendering) {
    glBindFramebuffer(GL_FRAMEBUFFER, this->aframebuffer.getFBO());
    glFramebufferTexture(GL_FRAMEBUFFER, attachments, this->cubeMap, 0);
    if(turnOffRendering) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

GLuint ACubeMap::getFBO(void) const {
    return this->aframebuffer.getFBO();
}

GLuint ACubeMap::getCubeMap(void) const {
    return this->cubeMap;
}

void ACubeMap::setMatrixAt(const GLuint index, const glm::mat4 matrix) {
    this->cubeMatrices[std::min(static_cast<int>(index), 5)] = matrix;
}

glm::mat4 ACubeMap::getMatrixAt(const GLuint index) const {
    return this->cubeMatrices[static_cast<int>(index)];
}

void ACubeMap::setMatricesUniform(const GLuint (&uniforms)[6]) const {
    for (unsigned int i = 0; i < 6; ++i) {
        glUniformMatrix4fv(uniforms[i], 1, GL_FALSE, glm::value_ptr(this->cubeMatrices[i]));
    }
}

////////////////////////

ADepthCubeMap::ADepthCubeMap(AFramebuffer& aframebuffer, GLuint width, GLuint height, GLuint type) : ACubeMap(aframebuffer, width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, type)
{ 
    this->setFramebufferParameters();
}

ADepthCubeMap::~ADepthCubeMap(void) {
    //TODO implement
}

void ADepthCubeMap::setFramebufferParameters(void) {
    ACubeMap::setFramebufferParameters(GL_DEPTH_ATTACHMENT, true);
}