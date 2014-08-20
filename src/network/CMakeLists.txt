# bernd.loerwald@itwm.fraunhofer.de

add_library (net connection.cpp)
target_link_libraries (net fhg-util ${Boost_SYSTEM_LIBRARY})

add_subdirectory (test)
