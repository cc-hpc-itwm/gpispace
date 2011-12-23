set(target pnet2eklmn)

add_executable (${target} "main.cpp")

target_link_libraries(${target} ${WE_LIBRARIES})
target_link_libraries(${target} ${FhgLog_LIBRARY})
target_link_libraries(${target} ${Boost_LIBRARIES})
target_link_libraries(${target} pthread)
target_link_libraries(${target} dl)

install (TARGETS ${target}
	RUNTIME
	DESTINATION bin
	COMPONENT applications
)
