#include "alight.hpp"

ALight::ALight(glm::vec3 position, glm::vec3 direction, glm::vec4 color, float intensity, bool directional) 
 : position(position), direction(direction), color(color), intensity(intensity), directional(directional), up(glm::vec3(0,1,0))
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

void ALight::setUp(const glm::vec3 up) {
    this->up.x = up.x;
    this->up.y = up.y;
    this->up.z = up.z;
}