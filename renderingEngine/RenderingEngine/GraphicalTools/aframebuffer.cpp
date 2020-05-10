#include "aframebuffer.hpp"

#include <stdio.h>
#include <limits>

AFramebuffer::AFramebuffer(void) 
{ 
    this->bufferShowFlag = std::numeric_limits<GLuint>::max();
}

AFramebuffer::AFramebuffer(GLfloat width, GLfloat height, GLint internalFormat, GLint format, GLint type) : width(width), height(height)
{
    FBO = 0;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    framebufferTexture = 0;
    glGenTextures(1, &framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
    generateRenderbuffer();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    {
        printf("Error creating FrameBuffer!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    this->bufferShowFlag = std::numeric_limits<GLuint>::max();
}

AFramebuffer::~AFramebuffer(void)
{
    glDeleteFramebuffers(1, &FBO);
}

GLuint AFramebuffer::getFBO(void) const
{
    return FBO;
}

GLuint AFramebuffer::getFramebufferTexture(void) const
{
    return framebufferTexture;
}

GLfloat AFramebuffer::getWidth(void) const 
{
    return width;
}

GLfloat AFramebuffer::getHeight(void) const 
{
    return height;
}

void AFramebuffer::bindBuffer(void) const {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void AFramebuffer::bindBuffer(GLuint showFlag) const 
{
    if(showFlag & this->bufferShowFlag) {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    }
}

void AFramebuffer::setViewport(void) const
{
    glViewport(0, 0, width, height);
}

void AFramebuffer::unbindBuffer(void) const 
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AFramebuffer::generateRenderbuffer(void) 
{
    RBO = 0;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
}

void AFramebuffer::setBufferShowFlag(GLuint bufferShowFlag) 
{
    this->bufferShowFlag = bufferShowFlag;
}


//////////

ADepthbuffer::ADepthbuffer(GLfloat width, GLfloat height) : AFramebuffer()
{
    this->width = width;
    this->height = height;
	glGenFramebuffers(1, &FBO);
	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebufferTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ADepthbuffer::~ADepthbuffer(void)
{
    //TODO
}

//////////

AGBuffer::AGBuffer(GLfloat width, GLfloat height) : width(width), height(height)
{
    GLenum err;
    if((err = glGetError()) != GL_NO_ERROR) {
        printf("!0 GLSTATUS! > Error: %d]\n", err);
    }

    FBO = 0;
    glGenFramebuffers(1, &FBO);
    if((err = glGetError()) != GL_NO_ERROR) {
        printf("!1 GLSTATUS! > Error: %d]\n", err);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    if((err = glGetError()) != GL_NO_ERROR) {
        printf("!2 GLSTATUS! > Error: %d]\n", err);
    }

    glGenTextures(1, &posTex);
    glBindTexture(GL_TEXTURE_2D, posTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTex, 0);
    if((err = glGetError()) != GL_NO_ERROR) {
        printf("!2 GLSTATUS! > Error: %d]\n", err);
    } else {
        printf("Generated texture [%d]\n", posTex);
    }

    glGenTextures(1, &norTex);
    glBindTexture(GL_TEXTURE_2D, norTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, norTex, 0);
    if((err = glGetError()) != GL_NO_ERROR) {
        printf("!2 GLSTATUS! > Error: %d]\n", err);
    } else {
        printf("Generated texture [%d]\n", norTex);
    }

    glGenTextures(1, &colTex);
    glBindTexture(GL_TEXTURE_2D, colTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, colTex, 0);
    if((err = glGetError()) != GL_NO_ERROR) {
        printf("!2 GLSTATUS! > Error: %d]\n", err);
    } else {
        printf("Generated texture [%d]\n", colTex);
    }
    
    gtextures = new GLuint[3] {posTex, norTex, colTex};

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &DBO);
    glBindRenderbuffer(GL_RENDERBUFFER, DBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DBO);

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        auto glstatus = glGetError();
        printf("Error in GBuffer - Framebuffer not complete! - [%d] [Error: %d]\n", fboStatus, glstatus);
    } else {
        printf("GBuffer created with success %d %d %d!\n", gtextures[0], gtextures[1], gtextures[2]);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

AGBuffer::~AGBuffer(void) {
    //TODO
}

GLuint AGBuffer::getFBO(void) const
{
    return FBO;
}

GLfloat AGBuffer::getWidth(void) const 
{
    return width;
}

GLfloat AGBuffer::getHeight(void) const 
{
    return height;
}

void AGBuffer::bindBuffer(void) const 
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void AGBuffer::setViewport(void) const
{
    glViewport(0, 0, width, height);
}

void AGBuffer::unbindBuffer(void) const 
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

const GLuint* AGBuffer::getTextures(void) const 
{
    return gtextures;
}