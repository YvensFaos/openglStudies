#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <luahandler.hpp>
#include <doctest/doctest.h>

#include <RenderingEngine/Core/alight.hpp>
#include <RenderingEngine/Core/acamera.hpp>
#include <RenderingEngine/Core/amodel.hpp>
#include <RenderingEngine/Utils/aluahelper.hpp>
#include <RenderingEngine/Utils/amacrohelper.hpp>


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

TEST_CASE("Simple Ambient Light reading.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    AAmbientLight* aambientLight = ALuaHelper::loadAmbientLightFromTable("ambient", &handler);

    glm::vec4 color = aambientLight->getColor();
    CHECK(color.r == 1.0000f);
    CHECK(color.g == 1.0000f);
    CHECK(color.b == 1.0000f);
    CHECK(color.a == 1.0000f);

    CHECK(aambientLight->getIntensity() == 0.05f);

    delete aambientLight;
}

TEST_CASE("Simple Camera position setup.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    ACamera* acamera = new ACamera();

    glm::vec3 position = acamera->getPos();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 0.0f);
    CHECK(position.z == 0.0f);

    ALuaHelper::setupCameraPosition("cameraPosition", acamera, &handler);
    position = acamera->getPos();
    CHECK(position.x == 1.0f);
    CHECK(position.y == 8.5f);
    CHECK(position.z ==17.0f);

    glm::vec3 direction = acamera->getDir();
    CHECK(direction.x == 0.0f);
    CHECK(direction.y == 0.0f);
    CHECK(direction.z ==-1.0f);

    glm::vec3 up = acamera->getUp();
    CHECK(up.x == 0.0f);
    CHECK(up.y == 0.9f);
    CHECK(up.z == 0.1f);

    glm::vec3 right = acamera->getRight();
    CHECK(right.x == 0.8f);
    CHECK(right.y == 0.2f);
    CHECK(right.z == 0.0f);

    delete acamera;
}

TEST_CASE("Simple Camera movement test.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    ACamera* acamera = new ACamera();

    glm::vec3 position = acamera->getPos();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 0.0f);
    CHECK(position.z == 0.0f);

    acamera->setDir(glm::vec3(0.0f, 0.0f, 1.0f));
    acamera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
    acamera->setRight(glm::vec3(1.0f, 0.0f, 0.0f));

    acamera->MoveForward(10.0f);
    position = acamera->getPos();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 0.0f);
    CHECK(position.z ==10.0f);

    acamera->MoveForward(-10.0f);
    position = acamera->getPos();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 0.0f);
    CHECK(position.z == 0.0f);

    acamera->MoveUp(2.0f);
    position = acamera->getPos();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 2.0f);
    CHECK(position.z == 0.0f);

    acamera->MoveForward(4.0f);
    position = acamera->getPos();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 2.0f);
    CHECK(position.z == 4.0f);

    acamera->MoveSideway(3.0f);
    position = acamera->getPos();
    CHECK(position.x == 3.0f);
    CHECK(position.y == 2.0f);
    CHECK(position.z == 4.0f);

    acamera->MoveSideway(-1.0f);
    position = acamera->getPos();
    CHECK(position.x == 2.0f);
    CHECK(position.y == 2.0f);
    CHECK(position.z == 4.0f);

    delete acamera;
}

TEST_CASE("Simple Camera zoom test.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    ACamera* acamera = new ACamera();

    CHECK(acamera->getZoom() == 45.0f);
    acamera->Zoom(90.0f);
    CHECK(acamera->getZoom() == 45.0f);
    acamera->Zoom(10.0f);
    CHECK(acamera->getZoom() == 55.0f);
    acamera->Zoom(-100.0f);
    CHECK(acamera->getZoom() == 55.0f);
    acamera->Zoom(-2.5f);
    CHECK(acamera->getZoom() == 52.5f);

    delete acamera;
}

TEST_CASE("Rotate Camera with mouse test.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    ACamera* acamera = new ACamera();
    acamera->RotateWithMouse(20.0f, 10.0f);

    glm::vec3 position = acamera->getPos();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 0.0f);
    CHECK(position.z == 0.0f);

    glm::vec3 direction = acamera->getDir();
    CHECK(fabs(direction.x -  0.33682f) < 0.01f);
    CHECK(fabs(direction.y -  0.17365f) < 0.01f);
    CHECK(fabs(direction.z - -0.92543f) < 0.01f);

    glm::vec3 up = acamera->getUp();
    CHECK(fabs(up.x - -0.05939f) < 0.01f);
    CHECK(fabs(up.y -  0.98481f) < 0.01f);
    CHECK(fabs(up.z -  0.16318f) < 0.01f);

    glm::vec3 right = acamera->getRight();
    CHECK(fabs(right.x -  0.93969f) < 0.01f);
    CHECK(fabs(right.y - -0.00000f) < 0.01f);
    CHECK(fabs(right.z -  0.34202f) < 0.01f);

    delete acamera;
}

TEST_CASE("List of Lights reading.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    std::vector<ALight*> lights = ALuaHelper::loadLightsFromTable("lights", &handler);
    ALight* alight;

    CHECK(lights.size() == 2);

    alight = lights[0];
    {
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
    }
    delete alight;

    alight = lights[1];
    {
        glm::vec3 position = alight->getPosition();
        CHECK(position.x == 0.0f);
        CHECK(position.y ==-2.0f);
        CHECK(position.z == 0.0f);
        glm::vec3 direction = alight->getDirection();
        CHECK(direction.x == 0.0f);
        CHECK(direction.y == 0.0f);
        CHECK(direction.z == 1.0f);
        glm::vec3 up = alight->getUp();
        CHECK(up.x == 0.0f);
        CHECK(up.y ==-1.0f);
        CHECK(up.z == 0.0f);
        glm::vec4 color = alight->getColor();
        CHECK(color.r == 0.8156f);
        CHECK(color.g == 0.2117f);
        CHECK(color.b == 0.9686f);
        CHECK(color.a == 0.7000f);
        CHECK(alight->getIntensity() == 100.0f);
        CHECK(alight->getSpecularPower() == 32.0f);
        CHECK(!alight->getDirectional());
    }
    delete alight;
}

TEST_CASE("Geometric Operations with Objects.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    AModel* amodel = new AModel(handler.getGlobalString("testModel"));

    glm::vec3 position;
    glm::mat4 matrix;

    position = amodel->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 0.0f);
    CHECK(position.z == 0.0f);

    matrix = amodel->getModelMatrix();
    CHECK(matrix[0][0] == 1.0f);    CHECK(matrix[0][1] == 0.0f);    CHECK(matrix[0][2] == 0.0f);    CHECK(matrix[0][3] == 0.0f);
    CHECK(matrix[1][0] == 0.0f);    CHECK(matrix[1][1] == 1.0f);    CHECK(matrix[1][2] == 0.0f);    CHECK(matrix[1][3] == 0.0f);
    CHECK(matrix[2][0] == 0.0f);    CHECK(matrix[2][1] == 0.0f);    CHECK(matrix[2][2] == 1.0f);    CHECK(matrix[2][3] == 0.0f);
    CHECK(matrix[3][0] == 0.0f);    CHECK(matrix[3][1] == 0.0f);    CHECK(matrix[3][2] == 0.0f);    CHECK(matrix[3][3] == 1.0f);

    amodel->translate(glm::vec3(0.0f, 1.0f, 0.0f));
    position = amodel->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 1.0f);
    CHECK(position.z == 0.0f);

    amodel->translate(glm::vec3(0.0f, 3.0f, 0.0f));
    position = amodel->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 4.0f);
    CHECK(position.z == 0.0f);

    amodel->rotate(glm::vec3(90.0f, 0.0f, 0.0f));
    position = amodel->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 4.0f);
    CHECK(position.z == 0.0f);

    matrix = amodel->getModelMatrix();
    CHECK(fabs(matrix[0][0] - 1.0f) < 0.01f);    CHECK(fabs(matrix[0][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[1][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][2] - 1.0f) < 0.01f);    CHECK(fabs(matrix[1][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[2][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][1] + 1.0f) < 0.01f);    CHECK(fabs(matrix[2][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[3][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][1] - 4.0f) < 0.01f);    CHECK(fabs(matrix[3][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][3] - 1.0f) < 0.01f);

    amodel->rotate(glm::vec3(-90.0f, 0.0f, 0.0f));
    position = amodel->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 4.0f);
    CHECK(position.z == 0.0f);

    matrix = amodel->getModelMatrix();
    CHECK(fabs(matrix[0][0] - 1.0f) < 0.01f);    CHECK(fabs(matrix[0][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[1][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][1] - 1.0f) < 0.01f);    CHECK(fabs(matrix[1][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[2][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][2] - 1.0f) < 0.01f);    CHECK(fabs(matrix[2][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[3][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][1] - 4.0f) < 0.01f);    CHECK(fabs(matrix[3][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][3] - 1.0f) < 0.01f);

    amodel->scale(glm::vec3(0.1f, 0.1f, 0.1f));
    position = amodel->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 4.0f);
    CHECK(position.z == 0.0f);

    amodel->scale(glm::vec3(2.0f, 2.0f, 2.0f));
    position = amodel->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 4.0f);
    CHECK(position.z == 0.0f);

    matrix = amodel->getModelMatrix();
    CHECK(fabs(matrix[0][0] - 0.2f) < 0.01f);    CHECK(fabs(matrix[0][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[1][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][1] - 0.2f) < 0.01f);    CHECK(fabs(matrix[1][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[2][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][2] - 0.2f) < 0.01f);    CHECK(fabs(matrix[2][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[3][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][1] - 4.0f) < 0.01f);    CHECK(fabs(matrix[3][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][3] - 1.0f) < 0.01f);

    amodel->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    position = amodel->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 0.0f);
    CHECK(position.z == 0.0f);
    matrix = amodel->getModelMatrix();
    CHECK(fabs(matrix[0][0] - 0.2f) < 0.01f);    CHECK(fabs(matrix[0][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[1][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][1] - 0.2f) < 0.01f);    CHECK(fabs(matrix[1][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[2][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][2] - 0.2f) < 0.01f);    CHECK(fabs(matrix[2][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[3][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][3] - 1.0f) < 0.01f);

    amodel->translate(glm::vec3(0.0f, 0.0f, 10.0f));
    position = amodel->getPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 0.0f);
    CHECK(position.z == 2.0f); //Because of the scale by 0.2

    matrix = amodel->getModelMatrix();
    CHECK(fabs(matrix[0][0] - 0.2f) < 0.01f);    CHECK(fabs(matrix[0][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[0][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[1][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][1] - 0.2f) < 0.01f);    CHECK(fabs(matrix[1][2] - 0.0f) < 0.01f);    CHECK(fabs(matrix[1][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[2][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[2][2] - 0.2f) < 0.01f);    CHECK(fabs(matrix[2][3] - 0.0f) < 0.01f);
    CHECK(fabs(matrix[3][0] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][1] - 0.0f) < 0.01f);    CHECK(fabs(matrix[3][2] - 2.0f) < 0.01f);    CHECK(fabs(matrix[3][3] - 1.0f) < 0.01f);

    delete amodel;
}