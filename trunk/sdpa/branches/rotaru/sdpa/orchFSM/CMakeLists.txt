add_subdirectory(SMC)
add_subdirectory(BSC)
add_dependencies(sdpa-orchFSM_BSC sdpa-events)
add_dependencies(sdpa-orchFSM_SMC sdpa-events)