add_executable(sdpa_nre main.cpp)
target_link_libraries(sdpa_nre sdpa)
install(TARGETS sdpa_nre DESTINATION bin)

add_executable(nre_worker worker.cpp)
target_link_libraries(nre_worker sdpa)
install(TARGETS nre_worker DESTINATION bin)
