set(PNETV_SOURCES
	main.cpp
)

add_executable(pnetv ${PNETV_SOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(pnetv fhg-revision)
target_link_libraries(pnetv pnetv-jpna)
target_link_libraries(pnetv pnetv-jpn)
target_link_libraries(pnetv pnet)
target_link_libraries(pnetv ${Boost_LIBRARIES})

install (TARGETS pnetv
	RUNTIME
	DESTINATION bin
	COMPONENT runtime
)

add_subdirectory(jpn)
add_subdirectory(jpna)

include(CTest)
if (BUILD_TESTING)
	add_subdirectory(tests)
endif()

# vim:set et sts=4 sw=4 nospell:
