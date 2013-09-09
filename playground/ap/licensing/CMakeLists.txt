find_package (NACL QUIET REQUIRED)
include_directories (${NACL_INCLUDE_DIR})

find_package (UTIL QUIET REQUIRED)
include_directories (${UTIL_INCLUDE_DIR})

add_executable (create-license create_license.cpp)
target_link_libraries (create-license ${NACL_LIBRARY})
target_link_libraries (create-license ${UTIL_LIBRARIES})

add_executable (create-keypair create_keypair.cpp)
target_link_libraries (create-keypair ${NACL_LIBRARY})
target_link_libraries (create-keypair ${UTIL_LIBRARIES})

add_executable (check-license check_license.cpp)
target_link_libraries (check-license ${NACL_LIBRARY})
target_link_libraries (check-license ${UTIL_LIBRARIES})
