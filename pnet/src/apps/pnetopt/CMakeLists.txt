find_package(LUA QUIET)

if (LUA_FOUND)

set(PNETOPT_SOURCES
	main.cpp
)

include_directories(SYSTEM ${LUA_INCLUDE_DIR})

add_executable(pnetopt ${PNETOPT_SOURCES})
target_link_libraries(pnetopt pnet)
target_link_libraries(pnetopt ${Boost_LIBRARIES})
target_link_libraries(pnetopt ${LUA_LIBRARY})
target_link_libraries(pnetopt ${CMAKE_DL_LIBS})

install (TARGETS pnetopt
	RUNTIME
	DESTINATION bin
	COMPONENT runtime
)
endif()
