#pragma once

#include <glm/glm.hpp>

class ACamera {
    private:
        glm::vec3 cameraPos;
        glm::vec3 cameraTarget;
        glm::vec3 cameraDirection;
        glm::vec3 up;
        glm::vec3 cameraRight;
        glm::vec3 cameraUp;

        float horizontalAngle;
        float verticalAngle;
        float zoom;
        float near;
        float far;

    public:
        ACamera(void);
        ACamera(glm::vec3 cameraPos, glm::vec3 cameraTarget, glm::vec3 up);
        ~ACamera(void);

        void MoveForward(float step);
        void MoveSideway(float step);
        void MoveUp(float step);
        void RotateWithMouse(float horizontalAngle, float verticalAngle);
        void Zoom(float zoom);

        glm::mat4 getView(void) const;
        const glm::vec3 getPos(void) const;
        const glm::vec3 getDir(void) const;
        const glm::vec3 getUp(void) const;
        const glm::vec3 getRight(void) const;
        float getZoom(void) const;
        float getNear(void) const;
        float getFar(void) const;

        void setPos  (const glm::vec3 value);
        void setDir  (const glm::vec3 value);
        void setUp   (const glm::vec3 value);
        void setRight(const glm::vec3 value);
};