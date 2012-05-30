set(PNETOPT_SOURCES
	main.cpp
)

find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

add_executable(pnetopt ${PNETOPT_SOURCES})
target_link_libraries(pnetopt ${Boost_LIBRARIES})
target_link_libraries(pnetopt ${LUA_LIBRARY})
target_link_libraries(pnetopt ${WE_LIBRARIES})

install (TARGETS pnetopt
	RUNTIME
	DESTINATION bin
	COMPONENT runtime
)
