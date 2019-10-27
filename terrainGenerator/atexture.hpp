#pragma once

#include <GL/glew.h>
#include <string>

struct ATextureData {
    GLuint width;
    GLuint height;
    float* data;

    ATextureData(GLuint width, GLuint height) : width(width), height(height) {
        data = new float[width * height * 4];
    }

    ~ATextureData() {
        delete [] data;
    }
};

class ATextureHolder {
    private:
        GLuint textureID;
        GLuint textureWidth;
        GLuint textureHeight;

    public:
        ATextureHolder(const std::string texturePath);
        ATextureHolder(const GLuint textureWidth, const GLuint textureHeight);
        ATextureHolder(const ATextureData& data);
        ~ATextureHolder(void);

        const GLuint getTextureID(void) const;
        void setTextureData(const ATextureData& textureData);
        void bindTexture(GLuint activateTextureIndex = 0) const;
        void unbindTexture(GLuint activateTextureIndex = 0) const;
    private:
        void initializeTexture(void);
};