add_subdirectory (client)
add_subdirectory (segment)
add_subdirectory (memory)
add_subdirectory (container)
add_subdirectory (global)

add_subdirectory (test)

extended_add_library (NAME gpi-space_url SOURCES "url.cpp" LIBRARIES fhg-util)
