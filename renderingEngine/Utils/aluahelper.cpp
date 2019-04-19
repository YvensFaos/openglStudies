#include "aluahelper.hpp"

#include "../amodel.hpp"
#include "../alight.hpp"
#include "luahandler.hpp"

std::vector<AModel*> ALuaHelper::loadModelsFromTable(std::string identifier, LuaHandler* luaHandler) 
{
    luaHandler->loadTable(identifier);
    GLuint tableSize = luaHandler->getLength();
    std::vector<AModel*> models;

    for(int i = 1; i <= tableSize; i++)
	{
        luaHandler->getTableFromTable(i);
        models.emplace_back(ALuaHelper::loadModel(luaHandler, true));
    }
    luaHandler->popTable();

    return models;
}

AModel* ALuaHelper::loadModelFromTable(std::string identifier, LuaHandler* luaHandler)
{
    luaHandler->loadTable(identifier);
    return ALuaHelper::loadModel(luaHandler, true);
}

AModel* ALuaHelper::loadModel(LuaHandler* luaHandler, bool popTable) 
{
    AModel* model = new AModel(luaHandler->getStringFromTable("file"));

    luaHandler->getTableFromTable("pos");
    glm::vec3 translateTo;
    translateTo.x = luaHandler->getNumberFromTable(1);
    translateTo.y = luaHandler->getNumberFromTable(2);
    translateTo.z = luaHandler->getNumberFromTable(3);
    model->translate(translateTo);
    luaHandler->popTable();

    luaHandler->getTableFromTable("rot");
	glm::vec3 rotateTo;
    rotateTo.x = luaHandler->getNumberFromTable(1);
    rotateTo.y = luaHandler->getNumberFromTable(2);
    rotateTo.z = luaHandler->getNumberFromTable(3);
    model->rotate(rotateTo);
    luaHandler->popTable();

    luaHandler->getTableFromTable("sca");
	glm::vec3 scaleTo;
    scaleTo.x = luaHandler->getNumberFromTable(1);
    scaleTo.y = luaHandler->getNumberFromTable(2);
    scaleTo.z = luaHandler->getNumberFromTable(3);
    model->scale(scaleTo);
    luaHandler->popTable();

    if(popTable) 
    {
        luaHandler->popTable();
    }

    return model;
}

ALight* ALuaHelper::loadLightFromTable(std::string identifier, LuaHandler* luaHandler) 
{
    luaHandler->loadTable(identifier);
    return ALuaHelper::loadLight(luaHandler, true);
}

ALight* ALuaHelper::loadLight(LuaHandler* luaHandler, bool popTable) 
{
    glm::vec3 lightPositionValue;
	glm::vec3 lightDirectionValue;
	glm::vec3 lightUpValue;
	glm::vec4 lightColorValue;
	float lightIntensityValue;
	bool lightDirectionalValue;

	luaHandler->getTableFromTable("pos");
	lightPositionValue.x = luaHandler->getNumberFromTable(1);
	lightPositionValue.y = luaHandler->getNumberFromTable(2);
	lightPositionValue.z = luaHandler->getNumberFromTable(3);
	luaHandler->popTable();

	luaHandler->getTableFromTable("dir");
	lightDirectionValue.x = luaHandler->getNumberFromTable(1);
	lightDirectionValue.y = luaHandler->getNumberFromTable(2);
	lightDirectionValue.z = luaHandler->getNumberFromTable(3);
	luaHandler->popTable();

	luaHandler->getTableFromTable("up");
	lightUpValue.x = luaHandler->getNumberFromTable(1);
	lightUpValue.y = luaHandler->getNumberFromTable(2);
	lightUpValue.z = luaHandler->getNumberFromTable(3);
	luaHandler->popTable();

	luaHandler->getTableFromTable("col");
	lightColorValue.x = luaHandler->getNumberFromTable(1);
	lightColorValue.y = luaHandler->getNumberFromTable(2);
	lightColorValue.z = luaHandler->getNumberFromTable(3);
	lightColorValue.w = luaHandler->getNumberFromTable(4);
	luaHandler->popTable();

	lightIntensityValue = luaHandler->getNumberFromTable("intensity");
	lightDirectionalValue = luaHandler->getBoolFromTable("directional");
	luaHandler->popTable();

	ALight* alight = new ALight(lightPositionValue, lightDirectionValue, lightColorValue, lightIntensityValue, lightDirectionalValue);
	alight->setUp(lightUpValue);

    return alight;
}