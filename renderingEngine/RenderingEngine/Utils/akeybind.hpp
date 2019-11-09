#pragma once

#include <functional>

class AKeyBind {
    private:
        const int key;
        std::function<void(int action, int mods)> keyAction;

    public:
        AKeyBind(int key, std::function<void(int action, int mods)> keyAction);
        AKeyBind(const AKeyBind& anotherKeyBind);
        ~AKeyBind() = default;

        int getKey(void) const;
        std::function<void(int action, int mods)> getKeyAction(void) const;
        void execute(int action, int mods) const;
};