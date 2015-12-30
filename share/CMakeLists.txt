# mirko.rahn@itwm.fraunhofer.de

set (PROCESS_LIBRARY_INSTALL_DESTINATION "libexec/gspc")
set (PROCESS_PUBLIC_HEADER_INSTALL_DESTINATION "include/process")

add_subdirectory (example)
add_subdirectory (lib)
add_subdirectory (man)
add_subdirectory (process)
add_subdirectory (seis)
add_subdirectory (selftest)
add_subdirectory (xsd)
