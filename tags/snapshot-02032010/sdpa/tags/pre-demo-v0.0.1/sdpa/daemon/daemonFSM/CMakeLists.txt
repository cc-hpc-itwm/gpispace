add_subdirectory(SMC)
add_dependencies(sdpa-daemonFSM_SMC sdpa-events)

add_subdirectory(BSC)
add_dependencies(sdpa-daemonFSM_BSC sdpa-events sdpa_daemon)
