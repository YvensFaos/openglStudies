#pragma once

#include <functional>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class AKeyBind {
    private:
        const int key;
        std::function<void(int action, int mods)> keyAction;

    public:
        AKeyBind(int key, std::function<void(int action, int mods)> keyAction);
        AKeyBind(const AKeyBind& anotherKeyBind);
        virtual ~AKeyBind() = default;

        int getKey(void) const;
        std::function<void(int action, int mods)> getKeyAction(void) const;
        void execute(int action, int mods) const;
};

class APressKeyBind : public AKeyBind {
    public:
        APressKeyBind(int key, std::function<void(int action, int mods)> keyAction, GLuint pressFlag = GLFW_PRESS);
        APressKeyBind(const APressKeyBind& anotherKeyBind);
        ~APressKeyBind() = default;
};

class AFlagTogglerKeyBind : public AKeyBind {
    public:
        AFlagTogglerKeyBind(int key, GLuint& flag, GLuint flagValue);
        AFlagTogglerKeyBind(const AFlagTogglerKeyBind& anotherKeyBind);
        ~AFlagTogglerKeyBind() = default;
};

class ABoolTogglerKeyBind : public AKeyBind {
 public:
        ABoolTogglerKeyBind(int key, bool& boolean);
        ABoolTogglerKeyBind(const ABoolTogglerKeyBind& anotherKeyBind);
        ~ABoolTogglerKeyBind() = default;
};