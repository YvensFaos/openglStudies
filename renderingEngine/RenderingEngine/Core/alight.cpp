#include "alight.hpp"

#include <string>

#define READ_UNIFORM(buffer, uniformName, lightUniform, shaderProgramme, appendText) \
    buffer = ""; \
    buffer.append(uniformName); \
    lightUniform = glGetUniformLocation(shaderProgramme, buffer.append(appendText).c_str());

AAmbientLight::AAmbientLight(glm::vec4 color, float intensity)
 : color(color), intensity(intensity)
 { }

AAmbientLight::~AAmbientLight(void) { }

glm::vec4 AAmbientLight::getColor(void) const {
    return this->color;
}

float AAmbientLight::getIntensity(void) const {
    return this->intensity;
}

void AAmbientLight::setColor(const glm::vec4 color) {
    this->color.x = color.x;
    this->color.y = color.y;
    this->color.z = color.z;
    this->color.w = color.w;
}

void AAmbientLight::setIntensity(const float intensity) {
    this->intensity = intensity;
}

//##### A LIGHT UNIFORM

ALightUniform ALightUniform::loadALightUniformFromProgramme(GLuint shaderProgramme, GLuint lightIndex, ALight& alight) {
    GLuint    lightPositionUniform = -1;
	GLuint   lightDirectionUniform = -1;
	GLuint       lightColorUniform = -1;
	GLuint   lightIntensityUniform = -1;
	GLuint lightDirectionalUniform = -1;
    GLuint    lightSpecularUniform = -1;

	char uniformName[64];
	bool isDirectional = alight.getDirectional();
    sprintf(uniformName, "%s[%d].", isDirectional ? "directionalLights" : "pointLights", lightIndex);
	std::string buffer;

    READ_UNIFORM(buffer, uniformName, lightPositionUniform, shaderProgramme, "position");
    READ_UNIFORM(buffer, uniformName, lightDirectionUniform, shaderProgramme, "direction");
    READ_UNIFORM(buffer, uniformName, lightColorUniform, shaderProgramme, "color");
    READ_UNIFORM(buffer, uniformName, lightIntensityUniform, shaderProgramme, "intensity");
    READ_UNIFORM(buffer, uniformName, lightDirectionalUniform, shaderProgramme, "directional");
    READ_UNIFORM(buffer, uniformName, lightSpecularUniform, shaderProgramme, "specularPower");

    return ALightUniform(lightPositionUniform, lightDirectionUniform, lightColorUniform, lightIntensityUniform, lightDirectionalUniform, lightSpecularUniform);
}

//##### A LIGHT

ALight::ALight(glm::vec3 position, glm::vec3 direction, glm::vec4 color, float intensity, bool directional) 
 : position(position), direction(direction), color(color), intensity(intensity), directional(directional), up(glm::vec3(0,1,0))
{ }

ALight::ALight(const ALight& anotherLight) 
 : position(anotherLight.getPosition()), direction(anotherLight.getDirection()), up(anotherLight.getUp()), color(anotherLight.getColor()),
   intensity(anotherLight.getIntensity()), specularPower(anotherLight.getSpecularPower()), directional(anotherLight.getDirectional())
{ }

ALight::~ALight(void) { }

glm::vec3 ALight::getPosition(void) const {
    return this->position;
}

glm::vec3 ALight::getDirection(void) const {
    return this->direction;
}

glm::vec3 ALight::getUp(void) const {
    return this->up;
}

glm::vec4 ALight::getColor(void) const {
    return this->color;
}

float ALight::getIntensity(void) const {
    return this->intensity;
}

bool ALight::getDirectional(void) const {
    return this->directional;
}

float ALight::getSpecularPower(void) const {
    return this->specularPower;
}

void ALight::setPosition(const glm::vec3 position) {
    this->position.x = position.x;
    this->position.y = position.y;
    this->position.z = position.z;
}

void ALight::setDirection(const glm::vec3 direction) {
    this->direction.x = direction.x;
    this->direction.y = direction.y;
    this->direction.z = direction.z;
}

void ALight::setColor(const glm::vec4 color) {
    this->color.x = color.x;
    this->color.y = color.y;
    this->color.z = color.z;
    this->color.w = color.w;
}

void ALight::setIntensity(const float intensity) {
    this->intensity = intensity;
}

void ALight::setSpecularPower(const float specularPower) {
    this->specularPower = specularPower;
}

void ALight::setUp(const glm::vec3 up) {
    this->up.x = up.x;
    this->up.y = up.y;
    this->up.z = up.z;
}

ALight& ALight::operator=(const ALight& anotherLight) {
    glm::vec3 position = anotherLight.getPosition();
    this->position.x = position.x;
    this->position.y = position.y;
    this->position.z = position.z;

    glm::vec3 direction = anotherLight.getDirection();
    this->direction.x = direction.x;
    this->direction.y = direction.y;
    this->direction.z = direction.z;

    glm::vec3 up = anotherLight.getUp();
    this->up.x = up.x;
    this->up.y = up.y;
    this->up.z = up.z;

    glm::vec4 color = anotherLight.getColor();
    this->color.x = color.x;
    this->color.y = color.y;
    this->color.z = color.z;
    this->color.w = color.w;

    this->setIntensity(anotherLight.getIntensity());
    this->setSpecularPower(anotherLight.getSpecularPower());
    this->directional = anotherLight.getDirectional();

    return *this;
}