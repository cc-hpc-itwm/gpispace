add_subdirectory(SMC)
add_dependencies(sdpa-jobFSM_SMC sdpa-events)

add_subdirectory(BSC)
add_dependencies(sdpa-jobFSM_BSC sdpa-events)
