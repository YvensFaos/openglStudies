#include "acamera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

ACamera::ACamera(void) :
    cameraPos(glm::vec3()), cameraTarget(glm::vec3(0.0f, 0.0f, 1.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)),
    zoom(45.0f), horizontalAngle(0.0f), verticalAngle(0.0f), near(0.1f), far(1000.0f)
{
    this->RotateWithMouse(-90.0, 0.0);
}

ACamera::ACamera(glm::vec3 cameraPos, glm::vec3 cameraTarget, glm::vec3 up) : 
    cameraPos(cameraPos), cameraTarget(cameraTarget), up(up), zoom(45.0f), horizontalAngle(0.0f), verticalAngle(0.0f),
    near(0.1f), far(1000.0f)
{
    this->RotateWithMouse(-90.0, 0.0);
}

ACamera::~ACamera(void)
{ }

void ACamera::MoveForward(float step)
{
    glm::vec3 forwardMovement = this->cameraDirection * step;
    this->cameraPos = this->cameraPos + forwardMovement;
}

void ACamera::MoveSideway(float step)
{
    glm::vec3 sidewayMovement = this->cameraRight * step;
    this->cameraPos = this->cameraPos + sidewayMovement;
}

void ACamera::MoveUp(float step)
{
    glm::vec3 upwardMovement = this->cameraUp * step;
    this->cameraPos = this->cameraPos + upwardMovement;
}

glm::mat4 ACamera::getView(void) const
{
    return glm::lookAt(this->cameraPos, this->cameraPos + this->cameraDirection, this->cameraUp);
}

const glm::vec3 ACamera::getPos(void) const
{
    return this->cameraPos;
}

const glm::vec3 ACamera::getDir(void) const
{
    return this->cameraDirection;
}

const glm::vec3 ACamera::getUp(void) const
{
    return this->cameraUp;
}

const glm::vec3 ACamera::getRight(void) const
{
    return this->cameraRight;
}

void ACamera::RotateWithMouse(float horizontalAngle, float verticalAngle)
{
    this->horizontalAngle += horizontalAngle;
    this->verticalAngle += verticalAngle;

    glm::vec3 front;
    front.x = cos(glm::radians(this->horizontalAngle)) * cos(glm::radians(this->verticalAngle));
    front.y = sin(glm::radians(this->verticalAngle));
    front.z = sin(glm::radians(this->horizontalAngle)) * cos(glm::radians(this->verticalAngle));

    this->cameraDirection = glm::normalize(front);
    this->cameraRight = glm::normalize(glm::cross(this->cameraDirection, this->up));
    this->cameraUp = glm::cross(this->cameraRight, this->cameraDirection);
}

void ACamera::CalculateRotationFromDirection(const glm::vec3 direction) 
{
    float verticalAngle = glm::degrees(asin(direction.y));
    int sign = (direction.x < 0.0 || direction.z < 0.0) ? -1 : 1;
    float horizontalAngle = sign * glm::degrees(acos((direction.x) / cos(glm::radians(verticalAngle))));

    this->horizontalAngle = horizontalAngle;
    this->verticalAngle = verticalAngle;
}

void ACamera::setMouseAngle(const glm::vec2 value)
{
    this->horizontalAngle = value.x;
    this->verticalAngle = value.y;
}

void ACamera::Zoom(float zoom)
{
    this->zoom += zoom;
    if(this->zoom >= 90.0 || this->zoom <= 0.0) 
    {
        this->zoom -= zoom;
    }
}

float ACamera::getZoom(void) const
{
    return this->zoom;
}

float ACamera::getNear(void) const
{
    return this->near;
}
float ACamera::getFar(void) const
{
    return this->far;
}

void ACamera::setPos(const glm::vec3 value)
{
    this->cameraPos.x = value.x;
    this->cameraPos.y = value.y;
    this->cameraPos.z = value.z;
}

void ACamera::setDir(const glm::vec3 value)
{
    this->cameraDirection.x = value.x;
    this->cameraDirection.y = value.y;
    this->cameraDirection.z = value.z;
}

void ACamera::setUp(const glm::vec3 value)
{
    this->cameraUp.x = value.x;
    this->cameraUp.y = value.y;
    this->cameraUp.z = value.z;
}

void ACamera::setRight(const glm::vec3 value)
{
    this->cameraRight.x = value.x;
    this->cameraRight.y = value.y;
    this->cameraRight.z = value.z;
}

const glm::vec2 ACamera::getAngles(void) const {
    return glm::vec2(this->horizontalAngle, this->verticalAngle);
}