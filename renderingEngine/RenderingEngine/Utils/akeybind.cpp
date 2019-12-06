#include "akeybind.hpp"

AKeyBind::AKeyBind(int key, std::function<void(int action, int mods)> keyAction) 
 : key(key), keyAction(keyAction)
 { }

AKeyBind::AKeyBind(const AKeyBind& anotherKeyBind)
 : key(anotherKeyBind.getKey()), keyAction(anotherKeyBind.getKeyAction())
 { }

int AKeyBind::getKey(void) const {
    return this->key;
}

std::function<void(int action, int mods)> AKeyBind::getKeyAction(void) const {
    return this->keyAction;
}

void AKeyBind::execute(int action, int mods) const {
    this->keyAction(action, mods);
}

///////////

APressKeyBind::APressKeyBind(int key, std::function<void(int action, int mods)> keyAction, GLuint pressFlag) : 
    AKeyBind(key, [&keyAction, pressFlag](int action, int mods) { 
        if(action & pressFlag) { keyAction(action, mods); }
    })
{ }

APressKeyBind::APressKeyBind(const APressKeyBind& anotherKeyBind) : AKeyBind(anotherKeyBind.getKey(), anotherKeyBind.getKeyAction()) 
{ }

/////////////

AFlagTogglerKeyBind::AFlagTogglerKeyBind(int key, GLuint& flag, GLuint flagValue) : 
    AKeyBind(key,[&flag, key, flagValue](int action, int mods) { 
		if(action == GLFW_PRESS) { if(mods == GLFW_MOD_SHIFT) { flag ^= flagValue; } else { flag |= flagValue; } }
		printf("Flag [%d]: %d.\n", key, flag);
    })
{ }

AFlagTogglerKeyBind::AFlagTogglerKeyBind(const AFlagTogglerKeyBind& anotherKeyBind) : 
    AKeyBind(anotherKeyBind.getKey(), anotherKeyBind.getKeyAction()) 
{ }