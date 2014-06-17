# bernd.loerwald@itwm.fraunhofer.de

add_definitions (-Wno-unused-parameter)

find_package (Qt4 4.7.1 COMPONENTS QtCore QtGui QtNetwork REQUIRED QUIET)

include (${QT_USE_FILE})

include_directories (${UTIL_INCLUDE_DIR})

include_directories (${CMAKE_SOURCE_DIR}/editor/src)

qt4_wrap_cpp (wrapped_cpps_server dummy_server.hpp OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})

  add_executable (dummy_server dummy_server.cpp ${wrapped_cpps_server})
  target_link_libraries (dummy_server ${QT_LIBRARIES} fhg_util_qt fhg-util)

#add_executable (serialize_without_default_ctor serialize_without_default_ctor.cpp)
#target_link_libraries (serialize_without_default_ctor ${Boost_SERIALIZATION_LIBRARY})


include_directories (${CMAKE_SOURCE_DIR})

add_library (net net/connection.cpp)
target_link_libraries (net fhg-util ${Boost_SYSTEM_LIBRARY})

add_library (rpc rpc/client.cpp rpc/server.cpp)
target_link_libraries ( rpc fhg-util net ${Boost_SERIALIZATION_LIBRARY})

add_executable (rif_client rif_client.cpp)
target_link_libraries (rif_client
  rpc
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_THREAD_LIBRARY}
)

add_executable (rif_server rif_server.cpp)
target_link_libraries (rif_server
  rpc
  fhg-util
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
)
