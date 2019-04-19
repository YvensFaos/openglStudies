#pragma once

#include <vector>
#include <string>

class LuaHandler;
class AModel;
class ALight;

class ALuaHelper {
    public:
        ///Load a vector of AModel pointers from a table with given identifier. Pop the table at the end.
        static std::vector<AModel*> loadModelsFromTable(std::string identifier, LuaHandler* luaHandler);
        ///Load a AModel from a table, popping it out of the stack after it finishes reading it.
        static AModel* loadModelFromTable(std::string identifier, LuaHandler* luaHandler);
        ///Load a AModel from a table on top of the stack. Optionally pop the table at the end. 
        static AModel* loadModel(LuaHandler* luaHandler, bool popTable = false);

        ///Load a ALight from a table, popping it out of the stack after it finishes reading it.
        static ALight* loadLightFromTable(std::string identifier, LuaHandler* luaHandler);
        ///Load a ALight from a table on top of the stack. Optionally pop the table at the end. 
        static ALight* loadLight(LuaHandler* luaHandler, bool popTable = false);
};