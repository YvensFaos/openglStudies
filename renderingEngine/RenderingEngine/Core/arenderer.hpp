#pragma once

#include <glm/glm.hpp>
#include <string>

class GLFWwindow;
class ACamera;

class ARenderer 
{
    public:
        std::string title;
        float width;
        float height;
    private:
        GLFWwindow* window;
        bool firstMouse = true;
        bool mouseIsClickingLeft = false;

        float shiftModPower = 1.0f;
        float deltaTime = 0.0f;
        float lastX;
        float lastY;
        float moveForce = 20.0f;
        float mouseSensitivity = 5.0;

        double accumulator = 0.0f;
        double currentTime = 0.0;
        double finishFrameTime = 0.0;

        char titleBuffer[196];

        glm::vec4 clearColor = glm::vec4(0.02f, 0.25f, 0.02f, 1.0f);

        ACamera* acamera;
    public:
        ARenderer(float width, float height, std::string title);
        ~ARenderer(void);

        void changeClearColor(glm::vec4 clearColor);

        ACamera* getCamera(void) const;

        void startFrame(void);
        void finishFrame(void);

        float getDeltaTime(void) const;
        double getAccumulator(void) const;
        bool isRunning(void) const;
    private:
        static void staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void staticMouseCallback(GLFWwindow* window, double xpos, double ypos);
        static void staticMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        void initialize(void);
        virtual void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        virtual void mouseCallback(GLFWwindow* window, double xpos, double ypos);
        virtual void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};