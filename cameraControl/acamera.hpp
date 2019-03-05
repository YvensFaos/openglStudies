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
        glm::mat4 view;

    public:
        ACamera(glm::vec3 cameraPos, glm::vec3 cameraTarget);
        ~ACamera(void);

        void MoveForward(float step);
        void MoveSideway(float step);
        void MoveUp(float step);

        glm::mat4 getView(void) const;
        const glm::vec3 getPos(void) const;
};