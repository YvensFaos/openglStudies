#pragma once

#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

class ANormalDebugger
{
    private:
        GLuint vertexShader;
        GLuint geometryShader;
        GLuint fragmentShader;
        GLuint programme;

        GLuint modelUniform;
        GLuint viewProjectionMatrixUniform;
        GLuint normalColorUniform;
        glm::vec4 normalColor;

        static std::string defaultVertexShader;
        static std::string defaultGeometryShader;
        static std::string defaultFragmentShader;

    public:
        ANormalDebugger(void);
        ANormalDebugger(std::string vertexShaderText, std::string geometryShaderText, std::string fragmentShaderText);
        ~ANormalDebugger(void);

        void setupForRendering(glm::mat4 viewProjectionMatrix) const;

        const GLuint getProgramme(void) const;
        const GLuint getModelUniformLocation(void) const;

        const glm::vec4 getNormalColor(void) const;
        void setNormalColor(const glm::vec4 normalColor);

    private:
        void initialize(std::string vertexShaderText, std::string geometryShaderText, std::string fragmentShaderText);
};