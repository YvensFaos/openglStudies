#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <luahandler.hpp>
#include <doctest/doctest.h>

TEST_CASE("Simple variable reading.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    CHECK(handler.getGlobalInteger("integerTest") == 2);
    CHECK(handler.getGlobalNumber("floatTest") == 2.0f);
    CHECK(handler.getGlobalString("stringTest").compare("hello") == 0.0f);
    CHECK(handler.getGlobalBoolean("boolTest") == false);
    CHECK(handler.getStackTop() == 0);
}

TEST_CASE("Table variable reading.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    handler.loadTable("complexTable");
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.getIntegerFromTable("value1") == 111);
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.getNumberFromTable("value2") == 20.0f);
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.getStringFromTable("value3").compare("hello") == 0);
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.getBoolFromTable("value4") == true);
    CHECK(handler.getStackTop() == 1);

    handler.getTableFromTable("value5");
    CHECK(handler.getStackTop() == 2);
    CHECK(handler.getIntegerFromTable("subvalue1") == 1111);
    CHECK(handler.getStackTop() == 2);
    CHECK(handler.getNumberFromTable("subvalue2") == 555.0);
    CHECK(handler.getStackTop() == 2);
    handler.popTable();
    CHECK(handler.getStackTop() == 1);
    handler.popTable();
    CHECK(handler.getStackTop() == 0);
}

TEST_CASE("Index table variable reading.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    handler.loadTable("indexedTable");
    CHECK(handler.getStackTop() == 1);

    CHECK(handler.getIntegerFromTable(1) == 1);
    CHECK(handler.getIntegerFromTable(5) == 5);
    CHECK(handler.getStringFromTable(6).compare("test") == 0);
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.getLength() == 6);

    handler.popTable();
    CHECK(handler.getStackTop() == 0);
}

TEST_CASE("Calling Functions.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    handler.getAndCallFunction("getValue", 1);
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.popInteger() == 10);
    CHECK(handler.getStackTop() == 0);

    CHECK(handler.getFunction("addNumbers"));
    handler.pushNumber(2.50f);
    handler.pushNumber(3.25f);
    CHECK(handler.getStackTop() == 3);
    handler.callFunctionFromStack(2, 1);
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.popNumber() == 5.75f);
    CHECK(handler.getStackTop() == 0);
}

TEST_CASE("Check reading functions.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    CHECK(handler.getFunction("addNumbers"));
    CHECK(handler.popTop() == 0);
    
    //Checking inexistent function
    CHECK(!handler.getFunction("adddNumbers"));
    CHECK(handler.getStackTop() == 0);
}

TEST_CASE("Fail safe while reading variables that are not in the table reading.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    CHECK(handler.getStackTop() == 0);
    handler.loadTable("missingTable");
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.getIntegerFromTable("value1") == 123);
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.getIntegerFromTable("value2") == -1);
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.getIntegerFromTable("value3") == 321);
    CHECK(handler.getStackTop() == 1);

    handler.popTable();
    CHECK(handler.getStackTop() == 0);
}

TEST_CASE("Test table checking.") {
    LuaHandler handler;
    handler.openFile("testLua.lua");

    CHECK(handler.getStackTop() == 0);
    handler.loadTable("missingTable");
    CHECK(handler.getStackTop() == 1);
    CHECK(handler.isTopOfStackATable());
    handler.popTable();
    CHECK(handler.getStackTop() == 0);

    handler.loadTable("complexTable");
    CHECK(handler.getStackTop() == 1);
    handler.getTableFromTable("value5");
    CHECK(handler.getStackTop() == 2);
    CHECK(handler.isTopOfStackATable());
    handler.popTable();
    CHECK(handler.getStackTop() == 1);
    handler.popTable();
    CHECK(handler.getStackTop() == 0);
}