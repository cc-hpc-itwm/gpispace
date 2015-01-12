add_subdirectory (type)
add_subdirectory (client)
add_subdirectory (segment)
add_subdirectory (memory)
add_subdirectory (container)
add_subdirectory (global)

add_subdirectory (test)

add_library (gpi-space_url url.cpp)
target_link_libraries (gpi-space_url fhg-util)
