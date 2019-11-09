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