find_package(LUA)

if (LUA_FOUND)

set(PNETOPT_SOURCES
	main.cpp
)

include_directories(${LUA_INCLUDE_DIR})

add_executable(pnetopt ${PNETOPT_SOURCES})
target_link_libraries(pnetopt we)
target_link_libraries(pnetopt ${Boost_LIBRARIES})
target_link_libraries(pnetopt ${LUA_LIBRARY})

install (TARGETS pnetopt
	RUNTIME
	DESTINATION bin
	COMPONENT runtime
)
endif()
