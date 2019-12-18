#include "agaussianquad.hpp"

#include "../Core/ashader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>

std::string AGaussianQuad::gaussianFragmentHeader = 
    "#version 400\n"
    "   in vec2 vuv;\n"
    "   const int GAUSSIAN_WEIGHT = {1};\n"
    "   const float factor = 1.0f;\n"
    "   uniform sampler2D textureUniform;\n"
    "   uniform float gaussianWeight[{2}];\n"
    "   out vec4 frag_colour;\n"
    "   void main() {\n"
    "       vec2 step = vec2(1.0f) / textureSize(textureUniform, 0);\n";

std::string AGaussianQuad::gaussianXFragment = 
    "   vec3 xPass = texture(textureUniform, vuv).rgb * gaussianWeight[0];\n"
    "   for(int i = 1; i < GAUSSIAN_WEIGHT; i++) {\n"
    "       xPass += texture(textureUniform, vuv + vec2(factor * +i * step.x, 0.0f)).rgb * gaussianWeight[i];\n"
    "       xPass += texture(textureUniform, vuv - vec2(factor * +i * step.x, 0.0f)).rgb * gaussianWeight[i];\n"
    "   }\n"
    "   frag_colour = vec4(xPass, 1.0);\n}\n";

std::string AGaussianQuad::gaussianYFragment = 
    "   vec3 yPass = texture(textureUniform, vuv).rgb * gaussianWeight[0];\n"
    "   for(int i = 1; i < GAUSSIAN_WEIGHT; i++) {\n"
    "       yPass += texture(textureUniform, vuv + vec2(0.0f, factor * +i * step.y)).rgb * gaussianWeight[i];\n"
    "       yPass += texture(textureUniform, vuv - vec2(0.0f, factor * +i * step.y)).rgb * gaussianWeight[i];\n"
    "   }\n"
    "   frag_colour = vec4(yPass, 1.0);\n}\n";

AGaussianQuad::AGaussianQuad(GLfloat width, GLfloat height, float gaussianSigma, int gaussianWeightLength, int gaussianIterations) : 
    width(width), height(height), framebufferX(width, height), framebufferY(width, height), gaussianWeightLength(gaussianWeightLength), gaussianIterations(gaussianIterations)
{
    std::string fragmentShaderText = AGaussianQuad::gaussianFragmentHeader;

    std::string gaussianWeightLengthString = std::to_string(gaussianWeightLength);
    fragmentShaderText.replace(fragmentShaderText.find("{1}"), gaussianWeightLengthString.length() + 1, gaussianWeightLengthString);
    fragmentShaderText.replace(fragmentShaderText.find("{2}"), gaussianWeightLengthString.length() + 1, gaussianWeightLengthString);
    
    std::string fragmentShaderX = std::string(fragmentShaderText).append(AGaussianQuad::gaussianXFragment);
    std::string fragmentShaderY = std::string(fragmentShaderText).append(AGaussianQuad::gaussianYFragment);

    this->initialize(ARenderQuad::defaultVertexShader, fragmentShaderX);

    secondFragmentShader = AShader::generateShader(fragmentShaderY, GL_FRAGMENT_SHADER);
    secondProgramme = AShader::generateProgram(vertexShader, fragmentShader);

	float sigma2 = (gaussianSigma * gaussianSigma);
	float coeff = 1.0 / glm::two_pi<float>() * sigma2;
	std::function<float(float)> gaussianFunction = 
        [coeff, sigma2](float value) { 
            return (float) (coeff * exp(-(value*value) / (2.0 * sigma2)));
        };

	weight = new float[gaussianWeightLength];
	bufferStrings = new std::string[gaussianWeightLength];

	float gaussianSum = 0.0f;

	weightUniformsX = new GLuint[gaussianWeightLength];
	weightUniformsY = new GLuint[gaussianWeightLength];

	char uniformName[20];
	for (unsigned int i = 0; i < gaussianWeightLength; i++)
	{
		weight[i] = gaussianFunction(static_cast<float>(i));
		gaussianSum += i == 0 ? weight[i] : 2 * weight[i];
	}

	for (unsigned int i = 0; i < gaussianWeightLength; i++)
	{
		weight[i] /= gaussianSum;
		sprintf(uniformName, "%s[%d]", "gaussianWeight", i);
		bufferStrings[i] = "";
		bufferStrings[i].append(uniformName);
		weightUniformsX[i] = glGetUniformLocation(programme, bufferStrings[i].c_str());
		weightUniformsY[i] = glGetUniformLocation(programme, bufferStrings[i].c_str());
		printf("GAUSSIAN WEIGHT [%d = %f].\n", i, weight[i]);
	}
}

AGaussianQuad::~AGaussianQuad(void) {
    //TODO
    delete[] weight;
    delete[] bufferStrings;
    delete[] weightUniformsX;
    delete[] weightUniformsY;
}

//Not sure about the constness of this method, since it affects the framebuffers
void AGaussianQuad::render(GLuint texture) const {
    for(unsigned int i = 0; i < gaussianIterations; i++) {
        framebufferX.bindBuffer();
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(programme);
            for (unsigned int i = 0; i < gaussianWeightLength; i++)
            {
                glUniform1f(weightUniformsX[i], weight[i]);
            }

            glActiveTexture(GL_TEXTURE0);
            if(i == 0) {
                glBindTexture(GL_TEXTURE_2D, texture);
            } else {
                glBindTexture(GL_TEXTURE_2D, framebufferY.getFramebufferTexture());
            }
            glUniform1i(textureUniform, 0);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
        framebufferX.unbindBuffer();

        framebufferY.bindBuffer();
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(secondProgramme);
            for (unsigned int i = 0; i < gaussianWeightLength; i++)
            {
                glUniform1f(weightUniformsY[i], weight[i]);
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, framebufferX.getFramebufferTexture());
            glUniform1i(textureUniform, 0);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
        framebufferY.unbindBuffer();
    }
}

GLuint AGaussianQuad::getGaussianBlurTexture(void) const {
    return framebufferY.getFramebufferTexture();
}