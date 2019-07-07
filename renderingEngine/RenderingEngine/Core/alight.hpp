#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

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
};

struct ALightUniform {
    public:
        GLuint    lightPositionUniform;
        GLuint   lightDirectionUniform;
        GLuint       lightColorUniform;
        GLuint   lightIntensityUniform;
        GLuint lightDirectionalUniform;

    public:
        ALightUniform(GLuint lightPositionUniform, GLuint lightDirectionUniform, GLuint lightColorUniform, GLuint lightIntensityUniform, GLuint lightDirectionalUniform) :
        lightPositionUniform(lightPositionUniform),  lightDirectionUniform(lightDirectionUniform), lightColorUniform(lightColorUniform), lightIntensityUniform(lightIntensityUniform), lightDirectionalUniform(lightDirectionalUniform) 
        { }
        ~ALightUniform(void) {}
        ALightUniform(const ALightUniform& copyFrom) {
            this->lightPositionUniform = copyFrom.lightPositionUniform;
            this->lightDirectionUniform = copyFrom.lightDirectionUniform;
            this->lightColorUniform = copyFrom.lightColorUniform;
            this->lightIntensityUniform = copyFrom.lightIntensityUniform;
            this->lightDirectionalUniform = copyFrom.lightDirectionalUniform;
        }

        void operator=(const ALightUniform& copyFrom) {
            this->lightPositionUniform = copyFrom.lightPositionUniform;
            this->lightDirectionUniform = copyFrom.lightDirectionUniform;
            this->lightColorUniform = copyFrom.lightColorUniform;
            this->lightIntensityUniform = copyFrom.lightIntensityUniform;
            this->lightDirectionalUniform = copyFrom.lightDirectionalUniform;
        }
};

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
};