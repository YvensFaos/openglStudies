#pragma once

#include <GL/glew.h>
#include <string>

struct ATextureData {
    const GLuint width;
    const GLuint height;
    const float* data;

    ATextureData(GLuint width, GLuint height, float* data) : width(width), height(height), data(data) { }

    ~ATextureData() {
        delete[] data;
    }
};

class ATextureHolder {
    private:
        const GLuint texture;

    public:
        ATextureHolder(const char *path, const std::string &directory, bool gamma = false, const GLint minFilter = GL_LINEAR, const GLint magFilter = GL_LINEAR, const GLint textureWrapS = GL_CLAMP_TO_BORDER, const GLint textureWrapT = GL_CLAMP_TO_BORDER);
        ATextureHolder(const ATextureData& textureData, const GLint minFilter = GL_LINEAR, const GLint magFilter = GL_LINEAR, const GLint textureWrapS = GL_CLAMP_TO_BORDER, const GLint textureWrapT = GL_CLAMP_TO_BORDER);
        ATextureHolder(const GLint texture);
        ~ATextureHolder(void);

        void bindTexture(GLint textureUnit = 0) const;
        void unbindTexture(GLint textureUnit = 0) const;
        GLuint getTextureID(void) const;

    private:
        GLuint initializeFromTextureData(const ATextureData& textureData, const GLint minFilter, const GLint magFilter, const GLint textureWrapS, const GLint textureWrapT);
};