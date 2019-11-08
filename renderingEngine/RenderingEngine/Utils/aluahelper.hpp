#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

class LuaHandler;
class AModel;
class ALight;
class AAmbientLight;
class ACamera;
class AFog;

class ALuaHelper {
    public:
        ///Load a vector of AModel pointers from a table with given identifier. Pop the table at the end.
        static std::vector<AModel> loadModelsFromTable(std::string identifier, LuaHandler& luaHandler);
        ///Load a AModel from a table, popping it out of the stack after it finishes reading it.
        static AModel loadModelFromTable(std::string identifier, LuaHandler& luaHandler);
        ///Load a AModel from a table on top of the stack. Optionally pop the table at the end. 
        static AModel loadModel(LuaHandler& luaHandler, bool popTable = false);

        ///Load a list of pointers of ALight from a table, popping it out of the stack after it finishes reading it.
        static std::vector<ALight> loadLightsFromTable(std::string identifier, LuaHandler& luaHandler);
        ///Load a ALight from a table, popping it out of the stack after it finishes reading it.
        static ALight loadLightFromTable(int index, LuaHandler&luaHandler);
        ///Load a ALight from a table, popping it out of the stack after it finishes reading it.
        static ALight loadLightFromTable(std::string identifier, LuaHandler& luaHandler);
        ///Load a ALight from a table on top of the stack. Optionally pop the table at the end. 
        static ALight loadLight(LuaHandler& luaHandler, bool popTable = false);
        ///Update a ALight properties using a function named [updateFunction] accessed by [luaHandler]
        static ALight& updateLight(LuaHandler& luaHandler, ALight& alight, std::string updateFunction, float deltaTime);

        ///Load camera position, up and right parameters from the cameraTable and set it to the acamera pointer.
        static void setupCameraPosition(std::string cameraTable, ACamera& acamera , LuaHandler& luaHandler);

        ///Load a AAmbientLight from a table, popping it out of the stack after it finises reading it.
        static AAmbientLight loadAmbientLightFromTable(std::string identifier, LuaHandler& luaHandler);

        ///Load a AFog from a table, popping it out of the stack after it finishes reading it.
        static AFog loadFogFromTable(std::string identifier, LuaHandler& luaHandler);

        ///Load a glm::vec4 from 4 consecutive values in a table by the identifier name.
        static glm::vec4 readVec4FromTable(std::string identifier, LuaHandler& luaHandler);
};