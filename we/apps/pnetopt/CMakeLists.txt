set(PNETOPT_SOURCES
	main.cpp
)

find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

add_executable(pnetopt ${PNETOPT_SOURCES})
target_link_libraries(pnetopt ${LUA_LIBRARY})

install (TARGETS pnetopt
	RUNTIME
	DESTINATION bin
	COMPONENT runtime
)
