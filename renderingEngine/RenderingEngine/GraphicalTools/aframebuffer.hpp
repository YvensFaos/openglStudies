#pragma once

#include <GL/glew.h>
#include "../Objects/atextureholder.hpp"

class AFramebuffer {
    protected:
        GLfloat width;
        GLfloat height;

        GLuint FBO;
        GLuint RBO;
        GLuint framebufferTexture;

        GLuint bufferShowFlag;
    protected:
        AFramebuffer(void);
    public:
        AFramebuffer(GLfloat width, GLfloat height, GLint internalFormat = GL_RGB, GLint format = GL_RGB, GLint type = GL_UNSIGNED_BYTE);
        ~AFramebuffer(void);

        GLuint getFBO(void) const;
        GLuint getFramebufferTexture(void) const;
        GLfloat getWidth(void) const;
        GLfloat getHeight(void) const;

        void bindBuffer(void) const;
        void bindBuffer(GLuint showFlag) const;
        void setViewport(void) const;
        void unbindBuffer(void) const;

        void setBufferShowFlag(GLuint bufferShowFlag);

    protected:
        void generateRenderbuffer(void);
};

class ADepthbuffer : public AFramebuffer{
    public:
        ADepthbuffer(GLfloat width, GLfloat height);
        ~ADepthbuffer(void);
};

class AGBuffer {
    protected:
        const GLfloat width;
        const GLfloat height;

        GLuint FBO;
        GLuint DBO;
        GLuint RBO;
        GLuint framebufferTexture;

        GLuint posTex;
        GLuint norTex;
        GLuint colTex;

        GLuint* gtextures;
    public:
        AGBuffer(GLfloat width, GLfloat height);
        ~AGBuffer(void);

        GLuint getFBO(void) const;
        GLfloat getWidth(void) const;
        GLfloat getHeight(void) const;
        const GLuint* getTextures(void) const;

        void bindBuffer(void) const;
        void setViewport(void) const;
        void unbindBuffer(void) const;
};