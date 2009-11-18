add_executable(aggregator main.cpp)
add_dependencies(aggregator sdpa-aggregator)

target_link_libraries(aggregator sdpa-aggregator)
target_link_libraries(aggregator sdpa-events)
target_link_libraries(aggregator sdpa-daemon)
target_link_libraries(aggregator sdpa-util)
target_link_libraries(aggregator sdpa)
target_link_libraries(aggregator seda)
target_link_libraries(aggregator seda-comm)

install(TARGETS aggregator DESTINATION bin)
