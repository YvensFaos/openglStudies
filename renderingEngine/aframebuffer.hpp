#pragma once

#include <GL/glew.h>

class AFramebuffer {
    protected:
        GLfloat width;
        GLfloat height;

        GLuint FBO;
        GLuint framebufferTexture;
    protected:
        AFramebuffer(void);
    public:
        AFramebuffer(GLfloat width, GLfloat height);
        ~AFramebuffer(void);

        GLuint getFBO(void) const;
        GLuint getFramebufferTexture(void) const;
        GLfloat getWidth(void) const;
        GLfloat getHeight(void) const;
};

class ADepthbuffer : public AFramebuffer{
    public:
        ADepthbuffer(GLfloat width, GLfloat height);
        ~ADepthbuffer(void);
};