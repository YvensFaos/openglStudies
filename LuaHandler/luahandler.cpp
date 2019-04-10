#include "luahandler.hpp"

LuaHandler::LuaHandler(void) {
    this->lua = luaL_newstate();
    luaL_openlibs(lua);
}

LuaHandler::~LuaHandler(void) {
    if(this->lua) {
        lua_close(lua);
    }
}

bool LuaHandler::openFile(std::string fileName) {
    luaL_loadfile(lua, fileName.c_str());
    if (lua_pcall(lua, 0, 0, 0) != 0) {
        // TMLogger::getInstance()->logError("Error at loading file %s. Message is: %s.", fileName.c_str(), lua_tostring(lua, -1));
        return false;
    }
    // TMLogger::getInstance()->logLine("Successfully loaded lua file named: %s", fileName.c_str());
    return true;
}

bool LuaHandler::getGlobalBoolean(std::string globalVariableName) {
    lua_getglobal(lua, globalVariableName.c_str());
    if(lua_isboolean(lua, -1)) {
        bool value = lua_toboolean(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to load boolean variable named: %s", globalVariableName.c_str());
    }
    return false;
}

int LuaHandler::getGlobalInteger(std::string globalVariableName) {
    if(lua_getglobal(lua, globalVariableName.c_str()) == LUA_TNUMBER) {
        int value = static_cast<int>(lua_tonumber(lua, -1));
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to load int variable named: %s", globalVariableName.c_str());
    }
    return -1;
}

float LuaHandler::getGlobalNumber(std::string globalVariableName) {
    lua_getglobal(lua, globalVariableName.c_str());
    if(lua_isnumber(lua, -1)) {
        float value = lua_tonumber(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to load float variable named: %s", globalVariableName.c_str());
    }
    return -1.0f;
}

std::string LuaHandler::getGlobalString(std::string globalVariableName) {
    lua_getglobal(lua, globalVariableName.c_str());
    if(lua_isstring(lua, -1)) {
        std::string value = lua_tostring(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to load string variable named: %s", globalVariableName.c_str());
    }
    return "\0";
}

bool LuaHandler::getBoolFromTable(std::string key) {
    lua_pushstring(lua, key.c_str());
    lua_gettable(lua, -2);

    if(lua_isboolean(lua, -1)) {
        bool value = lua_toboolean(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to load boolean variable with key: %s", key.c_str());
        return false;
    }
}

int LuaHandler::getIntegerFromTable(std::string key) {
    lua_pushstring(lua, key.c_str());
    lua_gettable(lua, -2);

    if(lua_isinteger(lua, -1)) {
        float value = lua_tointeger(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to load integer variable with key: %s", key.c_str());
        return -1;
    }
}

float LuaHandler::getNumberFromTable(int index) {
    lua_pushinteger(lua, index);
    lua_gettable(lua, -2);

    if(lua_isnumber(lua, -1)) {
        float value = lua_tonumber(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to load number variable with index: %d", index);
        return -1.0f;
    }
}

float LuaHandler::getNumberFromTable(std::string key) {
    lua_pushstring(lua, key.c_str());
    lua_gettable(lua, -2);

    if(lua_isnumber(lua, -1)) {
        float value = lua_tonumber(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to load number variable with key: %s", key.c_str());
        return -1.0f;
    }
}

std::string LuaHandler::getStringFromTable(std::string key) {
    lua_pushstring(lua, key.c_str());
    lua_gettable(lua, -2);

    if(lua_isstring(lua, -1)) {
        std::string value = lua_tostring(lua, -1); 
        lua_pop(lua, 1);
        return value;
    } else {
        if(lua_isnil(lua, -1)) {
            lua_pop(lua, 1);
        }
        // TMLogger::getInstance()->logError("Unable to load string variable with key: %s", key.c_str());
        return "\0";
    }
}

bool LuaHandler::getTableFromTable(int index) {
    lua_pushinteger(lua, index);
    lua_gettable(lua, -2);

    bool success = lua_istable(lua, -1);
    if(!success) {
        bool isNil = lua_isnil(lua, -1);
        // TMLogger::getInstance()->logError("Unable to read table indexed %d from current table. Top = %d. Top %s", index, this->getStackTop(), isNil ? "is nil. Removing current top." : "is not nil");
        if(isNil) {
            lua_pop(lua, 1);
        }
    }
    return success;
}

bool LuaHandler::getTableFromTable(std::string key) {
    lua_pushstring(lua, key.c_str());
    lua_gettable(lua, -2);

    bool success = lua_istable(lua, -1);
    if(!success) {
        bool isNil = lua_isnil(lua, -1);
        // TMLogger::getInstance()->logError("Unable to read table named %s from current table. Top = %d. Top %s", key.c_str(), this->getStackTop(), isNil ? "is nil. Removing current top." : "is not nil");
        if(isNil) {
            lua_pop(lua, 1);
        }
    }
    return success;
}

void LuaHandler::loadTable(std::string tableName) {
    lua_getglobal(lua, tableName.c_str());
    if(!lua_istable(lua, -1)) {
        // TMLogger::getInstance()->logError("Unable to read table named: %s", tableName.c_str());
    }
}

void LuaHandler::popTable(void) {
    if(lua_istable(lua, -1)) {
        lua_pop(lua, 1);
    } else {
        // TMLogger::getInstance()->logError("Attempt to pop a table failed. No table found in the stack top");
    }
}

void LuaHandler::getFunction(std::string functionName) {
    lua_getglobal(lua, functionName.c_str());
}

void LuaHandler::callFunctionFromStack(int parameters, int returns) {
    lua_call(lua, parameters, returns);
}

void LuaHandler::getAndCallFunction(std::string functionName, int returns) {
    lua_getglobal(lua, functionName.c_str());
    lua_call(lua, 0, returns);
}

void LuaHandler::pushBoolean(bool value) {
    lua_pushboolean(lua, value);
}

void LuaHandler::pushInteger(int value) {
    lua_pushinteger(lua, value);
}

void LuaHandler::pushNumber(float value) {
    lua_pushnumber(lua, value);
}

void LuaHandler::pushString(std::string value) {
    lua_pushstring(lua, value.c_str());
}

bool LuaHandler::popBoolean() {
    if(!lua_isnil(lua, -1)) {
        bool value = lua_toboolean(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to pop boolean value from top. Top is nil.");
    }
    return false;
}

int LuaHandler::popInteger() {
    if(!lua_isnil(lua, -1)) {
        int value = lua_tointeger(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to pop integer value from top. Top is nil.");
    }
    return -1;
}

float LuaHandler::popNumber() {
    if(!lua_isnil(lua, -1)) {
        float value = lua_tonumber(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to pop float value from top. Top is nil.");
    }
    return -1.0f;
}

std::string LuaHandler::popString() {
    if(!lua_isnil(lua, -1)) {
        std::string value = lua_tostring(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        // TMLogger::getInstance()->logError("Unable to pop string value from top. Top is nil.");
    }
    return "\0";
}

int LuaHandler::getStackTop(void) {
    return lua_gettop(lua);
}