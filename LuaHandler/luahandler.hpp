#pragma once

#include <lua.hpp>
#include <string>

class LuaHandler {
    public:

    private:
        lua_State* lua;

    public:
        LuaHandler(void);
        ~LuaHandler(void);

    public:
        bool openFile(std::string fileName);
        bool getGlobalBoolean(std::string globalVariableName);
        int getGlobalInteger(std::string globalVariableName);
        float getGlobalNumber(std::string globalVariableName);
        std::string getGlobalString(std::string globalVariableName);

        bool getBoolFromTable(std::string key);
        int getIntegerFromTable(std::string key);
        float getNumberFromTable(int index);
        float getNumberFromTable(std::string key);
        std::string getStringFromTable(std::string key);

        bool getTableFromTable(int index);
        bool getTableFromTable(std::string key);

        void getFunction(std::string functionName);
        void callFunctionFromStack(int parameters, int returns);
        void getAndCallFunction(std::string functionName, int returns);

        void pushBoolean(bool value);
        void pushInteger(int value);
        void pushNumber(float value);
        void pushString(std::string value);

        bool popBoolean();
        int popInteger();
        float popNumber();
        std::string popString();

        void loadTable(std::string tableName);
        void popTable(void);

        int getStackTop(void);
        int getLength(void);
};