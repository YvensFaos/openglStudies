#include "aframebuffer.hpp"

#include <stdio.h>

AFramebuffer::AFramebuffer(void) 
{ }

AFramebuffer::AFramebuffer(GLfloat width, GLfloat height, GLint internalFormat, GLint format) : width(width), height(height)
{
    FBO = 0;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    framebufferTexture = 0;
    glGenTextures(1, &framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
    generateRenderbuffer();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    {
        printf("Error creating FrameBuffer!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

AFramebuffer::~AFramebuffer(void)
{

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

void AFramebuffer::bindBuffer(void) const 
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
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

}