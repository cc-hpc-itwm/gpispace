# bernd.loerwald@itwm.fraunhofer.de

add_library (fhg_util_qt boost_connect_impl.cpp)
target_link_libraries (fhg_util_qt ${QT_LIBRARIES})

add_subdirectory (test)
