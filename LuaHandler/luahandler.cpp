#include "luahandler.hpp"

#define LUA_LOG "###LUA_LOG :"

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
        printf("%sError at loading file %s. Message is: %s.\r\n", LUA_LOG, fileName.c_str(), lua_tostring(lua, -1));
        return false;
    }
    printf("%sSuccessfully loaded lua file named: %s.\r\n", LUA_LOG, fileName.c_str());
    return true;
}

bool LuaHandler::getGlobalBoolean(std::string globalVariableName) {
    lua_getglobal(lua, globalVariableName.c_str());
    if(lua_isboolean(lua, -1)) {
        bool value = lua_toboolean(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        printf("%sUnable to load boolean variable named: %s.\r\n", LUA_LOG, globalVariableName.c_str());
        lua_pop(lua, 1);
    }
    return false;
}

int LuaHandler::getGlobalInteger(std::string globalVariableName) {
    if(lua_getglobal(lua, globalVariableName.c_str()) == LUA_TNUMBER) {
        int value = static_cast<int>(lua_tonumber(lua, -1));
        lua_pop(lua, 1);
        return value;
    } else {
        printf("%sUnable to load int variable named: %s.\r\n", LUA_LOG, globalVariableName.c_str());
        lua_pop(lua, 1);
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
        printf("%sUnable to load float variable named: %s.\r\n", LUA_LOG, globalVariableName.c_str());
        lua_pop(lua, 1);
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
        printf("%sUnable to load string variable named: %s.\r\n", LUA_LOG, globalVariableName.c_str());
        lua_pop(lua, 1);
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
        printf("%sUnable to load boolean variable with key: %s.\r\n", LUA_LOG, key.c_str());
        lua_pop(lua, 1);
        return false;
    }
}

int LuaHandler::getIntegerFromTable(std::string key) {
    lua_pushstring(lua, key.c_str());
    lua_gettable(lua, -2);

    if(lua_isinteger(lua, -1)) {
        int value = static_cast<int>(lua_tointeger(lua, -1));
        lua_pop(lua, 1);
        return value;
    } else {
        printf("%sUnable to load integer variable with key: %s.\r\n", LUA_LOG, key.c_str());
        lua_pop(lua, 1);
        return -1;
    }
}

int LuaHandler::getIntegerFromTable(int index) {
    lua_pushinteger(lua, index);
    lua_gettable(lua, -2);

    if(lua_isinteger(lua, -1)) {
        int value = static_cast<int>(lua_tointeger(lua, -1));
        lua_pop(lua, 1);
        return value;
    } else {
        printf("%sUnable to load integer variable with index: %d.\r\n", LUA_LOG, index);
        lua_pop(lua, 1);
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
        printf("%sUnable to load number variable with index: %d.\r\n", LUA_LOG, index);
        lua_pop(lua, 1);
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
        printf("%sUnable to load number variable with key: %s.\r\n", LUA_LOG, key.c_str());
        lua_pop(lua, 1);
        return -1.0f;
    }
}

std::string LuaHandler::getStringFromTable(int index) {
    lua_pushinteger(lua, index);
    lua_gettable(lua, -2);

    if(lua_isstring(lua, -1)) {
        std::string value = lua_tostring(lua, -1); 
        lua_pop(lua, 1);
        return value;
    } else {
        printf("%sUnable to load string variable with index: %d.\r\n", LUA_LOG, index);
        lua_pop(lua, 1);
        return "\0";
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
        printf("%sUnable to load string variable with key: %s.\r\n", LUA_LOG, key.c_str());
        lua_pop(lua, 1);
        return "\0";
    }
}

bool LuaHandler::getTableFromTable(int index) {
    lua_pushinteger(lua, index);
    lua_gettable(lua, -2);

    bool success = isTopOfStackATable();
    if(!success) {
        printf("%sUnable to read table indexed %d from current table.\r\n", 
        LUA_LOG, index);
        lua_pop(lua, 1);
    }
    return success;
}

bool LuaHandler::getTableFromTable(std::string key) {
    lua_pushstring(lua, key.c_str());
    lua_gettable(lua, -2);

    bool success = isTopOfStackATable();
    if(!success) {
        printf("%sUnable to read table named %s from current table.\r\n", 
        LUA_LOG, key.c_str());
        lua_pop(lua, 1);
    }
    return success;
}

bool LuaHandler::isTopOfStackATable(void) const {
    return lua_istable(lua, -1);
}

void LuaHandler::loadTable(std::string tableName) {
    lua_getglobal(lua, tableName.c_str());
    if(!isTopOfStackATable()) {
        printf("%sUnable to read table named: %s.\r\n", LUA_LOG, tableName.c_str());
    }
}

void LuaHandler::popTable(void) {
    if(isTopOfStackATable()) {
        lua_pop(lua, 1);
    } else {
        printf("%sAttempt to pop a table failed. No table found in the stack top.\r\n", LUA_LOG);
    }
}

bool LuaHandler::getFunction(std::string functionName) {
    lua_getglobal(lua, functionName.c_str());
    bool validFunction = lua_isfunction(lua, -1);
    if(!validFunction) {
        printf("%sAttempt to load function named \"%s\" failed.\r\n", LUA_LOG, functionName.c_str());
        lua_pop(lua, 1);
    }
    return validFunction;
}

int LuaHandler::callFunctionFromStack(int parameters, int returns) {
    return lua_pcall(lua, parameters, returns, 0);
}

/**
 * Get a global function and call it immediately using lua_pcall.
 * Returns the lua_pcall method return.
 * Results will be pushed to the stack.
 */
int LuaHandler::getAndCallFunction(std::string functionName, int returns) {
    lua_getglobal(lua, functionName.c_str());
    return lua_pcall(lua, 0, returns, 0);
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
        printf("%sUnable to pop boolean value from top. Top is nil.\r\n", LUA_LOG);
    }
    return false;
}

int LuaHandler::popInteger() {
    if(!lua_isnil(lua, -1)) {
        int value = lua_tointeger(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        printf("%sUnable to pop integer value from top. Top is nil.\r\n", LUA_LOG);
    }
    return -1;
}

float LuaHandler::popNumber() {
    if(!lua_isnil(lua, -1)) {
        float value = lua_tonumber(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        printf("%sUnable to pop float value from top. Top is nil.\r\n", LUA_LOG);
    }
    return -1.0f;
}

std::string LuaHandler::popString() {
    if(!lua_isnil(lua, -1)) {
        std::string value = lua_tostring(lua, -1);
        lua_pop(lua, 1);
        return value;
    } else {
        printf("%sUnable to pop string value from top. Top is nil.\r\n", LUA_LOG);
    }
    return "\0";
}

int LuaHandler::popTop(void) {
    if(this->getStackTop() > 0) {
        lua_pop(lua, 1);
    }

    return this->getStackTop();
}

int LuaHandler::getStackTop(void) const {
    return lua_gettop(lua);
}

///Get the length of the object on top of the stack.
int LuaHandler::getLength(void) const {
    return lua_rawlen(lua, -1);
}