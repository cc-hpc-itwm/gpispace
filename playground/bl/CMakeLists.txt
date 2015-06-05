# bernd.loerwald@itwm.fraunhofer.de

add_definitions (-Wno-unused-parameter)

include_directories (${CMAKE_SOURCE_DIR}/editor/src)

qt4_wrap_cpp (wrapped_cpps_server dummy_server.hpp OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})

  add_executable (dummy_server dummy_server.cpp ${wrapped_cpps_server})
  target_link_libraries (dummy_server ${QT_LIBRARIES} fhg_util_qt fhg-util Util::Generic)

#add_executable (serialize_without_default_ctor serialize_without_default_ctor.cpp)
#target_link_libraries (serialize_without_default_ctor Boost::serialization Util::Generic)
