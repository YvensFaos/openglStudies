#pragma once

#include <GL/glew.h>

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
        const GLuint textureWidth;
        const GLuint textureHeight;

    public:
        ATextureHolder(const GLuint textureWidth, const GLuint textureHeight);
        ATextureHolder(const ATextureData& data);
        ~ATextureHolder(void);

        const GLuint getTextureID(void) const;
        void setTextureData(const ATextureData& textureData);
        void bindTexture(void) const;
        void unbindTexture(void) const;
    private:
        void initializeTexture(void);
};