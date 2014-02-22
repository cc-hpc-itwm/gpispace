set(PNETV_SOURCES
	main.cpp
)

add_executable(pnetv ${PNETV_SOURCES})

target_link_libraries(pnetv fhg-revision)
target_link_libraries(pnetv pnetv-jpna)
target_link_libraries(pnetv pnetv-jpn)
target_link_libraries(pnetv pnet)
target_link_libraries(pnetv ${Boost_PROGRAM_OPTIONS_LIBRARY})

install (TARGETS pnetv
	RUNTIME
	DESTINATION bin
)

add_subdirectory(jpn)
add_subdirectory(jpna)

	add_subdirectory(tests)

# vim:set et sts=4 sw=4 nospell:
