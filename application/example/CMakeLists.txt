add_subdirectory(concurrent)
add_subdirectory(process)
add_subdirectory(capabilities)
add_subdirectory(capabilities_no_mandatory)
add_subdirectory(atomic)
add_subdirectory(inotify)

include (fhg_test_macros)

add_subdirectory(stresstest/test)
add_subdirectory(workerlist/test)
