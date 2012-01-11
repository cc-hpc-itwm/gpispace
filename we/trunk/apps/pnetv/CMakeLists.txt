set(target pnetv)

add_executable (${target} "main.cpp")

include_directories(".")

target_link_libraries(${target} ${WE_LIBRARIES})
target_link_libraries(${target} ${FhgLog_LIBRARY})
target_link_libraries(${target} ${Boost_LIBRARIES})
target_link_libraries(${target} pthread)
target_link_libraries(${target} dl)
target_link_libraries(${target} jpn)

install (TARGETS ${target}
	RUNTIME
	DESTINATION bin
	COMPONENT runtime
)

add_subdirectory(jpn)
