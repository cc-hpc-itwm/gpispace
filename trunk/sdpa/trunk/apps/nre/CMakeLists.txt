add_executable(sdpa_nre main.cpp)
target_link_libraries(sdpa_nre sdpa)
install(TARGETS sdpa_nre DESTINATION bin)
