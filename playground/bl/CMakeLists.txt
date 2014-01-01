# bernd.loerwald@itwm.fraunhofer.de

find_package (Qt4 4.7.1 COMPONENTS QtCore QtGui QtNetwork REQUIRED QUIET)

include (${QT_USE_FILE})

include_directories (${UTIL_INCLUDE_DIR})

include_directories (${CMAKE_SOURCE_DIR}/editor/src)

qt4_wrap_cpp (wrapped_cpps_server dummy_server.hpp OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})

if (ENABLE_PNET_EDITOR)
  add_executable (dummy_server dummy_server.cpp ${wrapped_cpps_server})
  target_link_libraries (dummy_server ${QT_LIBRARIES} fhg_util_qt fhg-util)
endif()
