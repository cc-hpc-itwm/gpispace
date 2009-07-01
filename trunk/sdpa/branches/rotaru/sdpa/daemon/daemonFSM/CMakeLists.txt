add_subdirectory(SMC)
add_subdirectory(BSC)
add_dependencies(sdpa-daemonFSM_BSC sdpa-events)
add_dependencies(sdpa-daemonFSM_SMC sdpa-events)