#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <luahandler.hpp>
#include <doctest/doctest.h>

#include <RenderingEngine/Core/alight.hpp>
#include <RenderingEngine/Utils/aluahelper.hpp>

TEST_CASE("Single Light reading.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    ALight* alight = ALuaHelper::loadLightFromTable("light", &handler);

    glm::vec3 position = alight->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 2.0f);
    CHECK(position.z == 0.0f);

    delete alight;
}