#include <iostream>

#include <lua.hpp>

int main() {
	lua_State *l = lua_open();

	luaL_openlibs(l);

	std::cerr << "This line in directly from C" << std::endl;
	luaL_dofile(l, "script.lua");
	std::cerr << "Back to C again" << std::endl;

	lua_close(l);

	return 0;
}

/* vim:set et sts=4 sw=4: */
