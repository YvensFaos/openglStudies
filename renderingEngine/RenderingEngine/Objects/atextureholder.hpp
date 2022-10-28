#pragma once

#include <GL/glew.h>
#include <algorithm>
#include <iterator>
#include <string>

struct ATextureData {
    const GLuint width;
    const GLuint height;
    const float* data;

    ATextureData(GLuint width, GLuint height, float* data) : width(width), height(height), data(ATextureData::copyRGBAData(width, height, data)) 
    { }

    ~ATextureData() {
        delete[] data;
    }

    static float* copyRGBAData(GLuint width, GLuint height, float* data) {
        unsigned int size = width * height * 4;
        float* buffer = new float[width * height * 4];
        for (unsigned int i = 0; i < size; i++)
        {
            buffer[i] = data[i];
        }
        return buffer;
    }
};

class ATextureHolder {
    private:
        const GLuint texture;

    public:
    ////glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureData.width, textureData.height, 0, GL_RGBA, GL_FLOAT, textureData.data);
        ATextureHolder(const GLfloat width, const GLfloat height, const GLint internalFormat, const GLenum format, const GLenum type, const GLint minFilter = GL_LINEAR, const GLint magFilter = GL_LINEAR);
        ATextureHolder(const char *path, const std::string &directory, bool gamma = false, const GLint minFilter = GL_LINEAR, const GLint magFilter = GL_LINEAR, const GLint textureWrapS = GL_CLAMP_TO_BORDER, const GLint textureWrapT = GL_CLAMP_TO_BORDER, const GLint internalFormat = GL_RGBA, const GLenum format = GL_RGBA);
        ATextureHolder(const ATextureData& textureData, const GLint minFilter = GL_LINEAR, const GLint magFilter = GL_LINEAR, const GLint textureWrapS = GL_CLAMP_TO_BORDER, const GLint textureWrapT = GL_CLAMP_TO_BORDER, const GLint internalFormat = GL_RGBA, const GLenum format = GL_RGBA);
        ATextureHolder(const GLint texture);
        ~ATextureHolder(void);

        void bindTexture(GLint textureUnit = 0) const;
        void unbindTexture(GLint textureUnit = 0) const;
        GLuint getTextureID(void) const { return this->texture; };

    private:
        GLuint generateTextId(void);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureData.width, textureData.height, 0, GL_RGBA, GL_FLOAT, textureData.data);
        GLuint initializeFromTextureData(const ATextureData& textureData, const GLint minFilter, const GLint magFilter, const GLint textureWrapS, const GLint textureWrapT, const GLint internalFormat, const GLenum format);
};