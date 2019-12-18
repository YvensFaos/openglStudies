#pragma once

#include "arenderquad.hpp"
#include "../GraphicalTools/aframebuffer.hpp"

class AGaussianQuad : private ARenderQuad {
    private:
        GLfloat width; 
        GLfloat height;
        GLuint secondFragmentShader;
        GLuint secondProgramme;

        int gaussianIterations;
        int gaussianWeightLength;
        std::vector<GLfloat> gaussianWeight;

        AFramebuffer framebufferX;
        AFramebuffer framebufferY;

        float* weight;
        std::string* bufferStrings;
        GLuint* weightUniformsX;
        GLuint* weightUniformsY;
    private:
        static std::string gaussianFragmentHeader;
        static std::string gaussianXFragment;
        static std::string gaussianYFragment;
    public:
        AGaussianQuad(GLfloat width, GLfloat height, float gaussianSigma, int gaussianWeightLength, int gaussianIterations = 1);
        ~AGaussianQuad(void);

        void render(GLuint texture) const;
        GLuint getGaussianBlurTexture(void) const;
};