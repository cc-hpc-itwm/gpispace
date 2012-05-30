#include <iostream>

#include <boost/format.hpp>

#include <lua.hpp>

class Test {
    int value_;

    public:

    Test(int value): value_(value) {}

    int value() const { return value_; }
};

int Test__tostring(lua_State *L) {
    Test *object = (Test *)lua_touserdata(L, -1);
    assert(object != NULL);

    lua_pushstring(L, (boost::format("Test with value = %d") % object->value()).str().c_str());

    return 1;
}

int pnet(lua_State *L)
{
    static int x = 0;

    // Memory leak here.
    lua_pushlightuserdata(L, new Test(x++));

    if (luaL_newmetatable(L, "pnetopt.Test")) {
        static const struct luaL_Reg metatable[] = {
            { "__tostring", Test__tostring },
            { NULL, NULL }
        };
        luaL_register(L, NULL, metatable);
    }
    lua_setmetatable(L, -2);

    return 1;
}

void report_errors(lua_State *L, int status) {
    if (status != 0) {
        std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1); // remove error message
    }
}

int main() {
    lua_State *L = lua_open();

    luaL_openlibs(L);

    lua_register(L, "pnet", pnet);

    std::cerr << "This line in directly from C" << std::endl;

    int status = luaL_dofile(L, "script.lua");

    std::cerr << std::endl << "Back to C again" << std::endl;

    report_errors(L, status);

    lua_close(L);

    return 0;
}

/* vim:set et sts=4 sw=4: */
