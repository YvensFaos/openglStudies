#pragma once

#include <glm/glm.hpp>
#include <string>
#include <map>
#include <functional>

#include "../Utils/akeybind.hpp"

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
        bool firstMouse;
        bool mouseIsClickingLeft;

        float shiftModPower;
        float deltaTime;
        float lastX;
        float lastY;
        float moveForce;
        float mouseSensitivity;

        double accumulator;
        double currentTime;
        double finishFrameTime;

        char titleBuffer[196];
        glm::vec4 clearColor;
        ACamera* acamera;

        std::map<int, std::unique_ptr<AKeyBind>> keysMap;
    public:
        ARenderer(float width, float height, std::string title);
        ~ARenderer(void);

        void changeClearColor(glm::vec4 clearColor);

        ACamera& getCamera(void) const;

        void startFrame(void);
        void finishFrame(void);

        float getDeltaTime(void) const;
        double getAccumulator(void) const;
        bool isRunning(void) const;
        void setCullFaces(bool activate);
        void addKeybind(AKeyBind akeyBind);
    private:
        static void staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void staticMouseCallback(GLFWwindow* window, double xpos, double ypos);
        static void staticMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        void initialize(void);
        virtual void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        virtual void mouseCallback(GLFWwindow* window, double xpos, double ypos);
        virtual void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};