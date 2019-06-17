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

    glm::vec3 direction = alight->getDirection();
    CHECK(direction.x ==  0.0f);
    CHECK(direction.y ==  0.0f);
    CHECK(direction.z == -1.0f);

    glm::vec3 up = alight->getUp();
    CHECK(up.x == 0.0f);
    CHECK(up.y == 1.0f);
    CHECK(up.z == 0.0f);

    glm::vec4 color = alight->getColor();
    CHECK(color.r == 0.9686f);
    CHECK(color.g == 0.8156f);
    CHECK(color.b == 0.2117f);
    CHECK(color.a == 1.0000f);

    CHECK(alight->getIntensity() == 80.0f);
    CHECK(alight->getSpecularPower() == 0.0f);
    CHECK(alight->getDirectional());

    delete alight;
}