#pragma once

#include <glm/glm.hpp>

class ACamera {
    private:
        glm::vec3 cameraPos;
        glm::vec3 cameraTarget;
        glm::vec3 cameraDirection;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 cameraRight;
        glm::vec3 cameraUp;

        float horizontalAngle;
        float verticalAngle;
        float zoom;

    public:
        ACamera(glm::vec3 cameraPos, glm::vec3 cameraTarget);
        ~ACamera(void);

        void MoveForward(float step);
        void MoveSideway(float step);
        void MoveUp(float step);
        void RotateWithMouse(float horizontalAngle, float verticalAngle);
        void Zoom(float zoom);

        glm::mat4 getView(void) const;
        const glm::vec3 getPos(void) const;
        float getZoom(void) const;
};