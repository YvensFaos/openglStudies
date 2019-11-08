#pragma once

#include <GL/glew.h>

class AFramebuffer {
    protected:
        GLfloat width;
        GLfloat height;

        GLuint FBO;
        GLuint RBO;
        GLuint framebufferTexture;
    protected:
        AFramebuffer(void);
    public:
        AFramebuffer(GLfloat width, GLfloat height, GLint internalFormat = GL_RGB, GLint format = GL_RGB);
        ~AFramebuffer(void);

        GLuint getFBO(void) const;
        GLuint getFramebufferTexture(void) const;
        GLfloat getWidth(void) const;
        GLfloat getHeight(void) const;

        void bindBuffer(void) const;
        void setViewport(void) const;
        void unbindBuffer(void) const;

    protected:
        void generateRenderbuffer(void);
};

class ADepthbuffer : public AFramebuffer{
    public:
        ADepthbuffer(GLfloat width, GLfloat height);
        ~ADepthbuffer(void);
};