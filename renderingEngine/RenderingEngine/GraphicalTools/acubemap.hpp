#pragma once

#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

class AFramebuffer;

class ACubeMap {
    private:
        AFramebuffer& aframebuffer;
    protected:
        glm::mat4 cubeMatrices[6];
        GLuint cubeMap;
    public:
        ACubeMap(AFramebuffer& aframebuffer, GLuint width = 1024, GLuint height = 1024, GLuint internalFormat = GL_DEPTH_COMPONENT, GLuint format = GL_DEPTH_COMPONENT, GLuint type = GL_FLOAT);
        virtual ~ACubeMap(void);

        void setFramebufferParameters(GLuint attachments, bool turnOffRendering = false);

        GLuint getCubeMap(void) const;
        GLuint getFBO(void) const;

        void setMatrixAt(const GLuint index, const glm::mat4 matrix);
        glm::mat4 getMatrixAt(const GLuint index) const;
        void setMatricesUniform(const GLuint (&uniforms)[6]) const;
};

class ADepthCubeMap : public ACubeMap {
    public:
        ADepthCubeMap(AFramebuffer& aframebuffer, GLuint width = 1024, GLuint height = 1024, GLuint type = GL_FLOAT);
        ~ADepthCubeMap(void);

    private:
        void setFramebufferParameters();
};