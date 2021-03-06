#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class AAmbientLight {
    private:
        glm::vec4 color;
        float intensity; 
    
    public:
        AAmbientLight(glm::vec4 color, float intensity);
        ~AAmbientLight(void);

        glm::vec4 getColor(void) const;
        float getIntensity(void) const;
        void setColor(const glm::vec4 color);
        void setIntensity(const float intensity);

        void setupUniforms(const GLuint ambientLightColorUniform, const GLuint ambientLightIntensityUniform) const;
};

struct ALightUniform;

class ALight {
    private:
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 up;
        glm::vec4 color;
        float intensity; 
        float specularPower;
        bool directional;

    public:
        ALight(glm::vec3 position, glm::vec3 direction, glm::vec4 color, float intensity, bool directional = true);
        ALight(const ALight& anotherLight);
        ~ALight(void);

        glm::vec3 getPosition(void) const;
        glm::vec3 getDirection(void) const;
        glm::vec3 getUp(void) const;
        glm::vec4 getColor(void) const;
        float getIntensity(void) const;
        float getSpecularPower(void) const;
        bool getDirectional(void) const;

        void setPosition(const glm::vec3 position);
        void setDirection(const glm::vec3 direction);
        void setUp(const glm::vec3 up);
        void setColor(const glm::vec4 color);
        void setIntensity(const float intensity);
        void setSpecularPower(const float specularPower);

        void setupUniforms(GLuint lightPositionUniform, GLuint lightDirectionUniform, GLuint lightColorUniform, GLuint lightIntensityUniform, GLuint lightDirectionalUniform, GLuint lightSpecularUniform = -1);
        void setupUniforms(ALightUniform alightUniform);

        ALight& operator=(const ALight& anotherLight);

        void log(void) const;
};

struct ALightUniform {
    public:
        GLuint    lightPositionUniform;
        GLuint   lightDirectionUniform;
        GLuint       lightColorUniform;
        GLuint   lightIntensityUniform;
        GLuint    lightSpecularUniform;
        GLuint lightDirectionalUniform;

    public:
        ALightUniform(GLuint lightPositionUniform, GLuint lightDirectionUniform, GLuint lightColorUniform, GLuint lightIntensityUniform, GLuint lightDirectionalUniform, GLuint lightSpecularUniform = -1) :
        lightPositionUniform(lightPositionUniform),  lightDirectionUniform(lightDirectionUniform), lightColorUniform(lightColorUniform), lightIntensityUniform(lightIntensityUniform), lightDirectionalUniform(lightDirectionalUniform), lightSpecularUniform(lightSpecularUniform)
        { }
        
        ALightUniform(const ALightUniform& copyFrom) {
            this->lightPositionUniform = copyFrom.lightPositionUniform;
            this->lightDirectionUniform = copyFrom.lightDirectionUniform;
            this->lightColorUniform = copyFrom.lightColorUniform;
            this->lightIntensityUniform = copyFrom.lightIntensityUniform;
            this->lightDirectionalUniform = copyFrom.lightDirectionalUniform;
            this->lightSpecularUniform = copyFrom.lightSpecularUniform;
        }
        ~ALightUniform(void) {}

        ALightUniform& operator=(const ALightUniform& copyFrom) {
            if(this != &copyFrom) {
                this->lightPositionUniform = copyFrom.lightPositionUniform;
                this->lightDirectionUniform = copyFrom.lightDirectionUniform;
                this->lightColorUniform = copyFrom.lightColorUniform;
                this->lightIntensityUniform = copyFrom.lightIntensityUniform;
                this->lightDirectionalUniform = copyFrom.lightDirectionalUniform;
                this->lightSpecularUniform = copyFrom.lightSpecularUniform;
            }
            return *this;
        }

    public:
        static ALightUniform loadALightUniformFromProgramme(GLuint shaderProgramme, GLuint lightIndex, const ALight& alight);
        static ALightUniform loadALightUniformFromProgrammeWithName(GLuint shaderProgramme, GLuint lightIndex, const ALight& alight, std::string name);
};