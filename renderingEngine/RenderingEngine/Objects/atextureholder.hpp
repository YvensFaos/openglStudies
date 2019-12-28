#pragma once

#include <GL/glew.h>
#include <string>

class ATextureHolder {
    private:
        const GLuint texture;

    public:
        ATextureHolder(const char *path, const std::string &directory, bool gamma = false, const GLint minFilter = GL_LINEAR, const GLint magFilter = GL_LINEAR, const GLint textureWrapS = GL_CLAMP_TO_BORDER, const GLint textureWrapT = GL_CLAMP_TO_BORDER);
        ATextureHolder(const GLint texture);
        ~ATextureHolder(void);

        void bindTexture(GLint textureUnit = 0) const;
        GLuint getTextureID(void) const;

    private:
};