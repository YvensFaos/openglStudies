#include "acamera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

ACamera::ACamera(glm::vec3 cameraPos, glm::vec3 cameraTarget) : 
    cameraPos(cameraPos), cameraTarget(cameraTarget)
{
    this->cameraDirection = glm::normalize(this->cameraPos - this->cameraTarget);
    this->up = glm::vec3(0.0f, 1.0f, 0.0f); 
    this->cameraRight = glm::normalize(glm::cross(this->up, this->cameraDirection));
    this->cameraUp = glm::cross(this->cameraDirection, this->cameraRight);

    this->view = glm::lookAt(this->cameraPos, this->cameraTarget, this->cameraUp);
}

ACamera::~ACamera(void)
{ }

void ACamera::MoveForward(float step)
{
    glm::vec3 forwardMovement = this->cameraDirection * step;
    this->cameraPos = this->cameraPos + forwardMovement;
    this->cameraTarget = this->cameraTarget + forwardMovement;
    this->cameraDirection = glm::normalize(this->cameraPos - this->cameraTarget);
    this->view = glm::lookAt(this->cameraPos, this->cameraTarget, this->cameraUp);
}

void ACamera::MoveSideway(float step)
{
    glm::vec3 sidewayMovement = this->cameraRight * step;
    this->cameraPos = this->cameraPos + sidewayMovement;
    this->cameraTarget = this->cameraTarget + sidewayMovement;
    this->cameraDirection = glm::normalize(this->cameraPos - this->cameraTarget);
    this->view = glm::lookAt(this->cameraPos, this->cameraTarget, this->cameraUp);
}

void ACamera::MoveUp(float step)
{
    glm::vec3 upwardMovement = this->cameraUp * step;
    this->cameraPos = this->cameraPos + upwardMovement;
    this->cameraTarget = this->cameraTarget + upwardMovement;
    this->cameraDirection = glm::normalize(this->cameraPos - this->cameraTarget);
    this->view = glm::lookAt(this->cameraPos, this->cameraTarget, this->cameraUp);
}

glm::mat4 ACamera::getView(void) const
{
    return this->view;
}

const glm::vec3 ACamera::getPos(void) const
{
    return this->cameraPos;
}