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

//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        void changeTextureParameter(GLint textureParameter, GLint textureParameterValue);

    protected:
        void generateRenderbuffer(void);
};

class ADepthbuffer : public AFramebuffer{
    public:
        ADepthbuffer(GLfloat width, GLfloat height);
        ~ADepthbuffer(void);
};

class AGBuffer {
    private:
        const GLfloat width;
        const GLfloat height;

        GLuint FBO;
        GLuint DBO;

        const ATextureHolder positionTexture;
        const ATextureHolder normalTexture;
        const ATextureHolder colorTexture;
    public:
        AGBuffer(GLfloat width, GLfloat height);
        ~AGBuffer(void);

        const GLuint getFBO(void) const { return FBO; }
        const GLfloat getWidth(void) const { return width; }
        const GLfloat getHeight(void) const { return height; }
        
        const GLuint getPositionTextureID(void) const { return positionTexture.getTextureID(); }
        const GLuint getNormalTextureID(void) const { return normalTexture.getTextureID(); }
        const GLuint getColorTextureID(void) const { return colorTexture.getTextureID(); }

        void bindBuffer(void) const;
        void setViewport(void) const;
        void unbindBuffer(void) const;
};