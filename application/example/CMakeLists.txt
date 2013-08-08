add_subdirectory(concurrent)
add_subdirectory(process)
add_subdirectory(failing)
add_subdirectory(capabilities)
add_subdirectory(capabilities_no_mandatory)
add_subdirectory(atomic)
add_subdirectory(inotify)
add_subdirectory(tutorial)

include (fhg_test_macros)

add_subdirectory(stresstest/test)
