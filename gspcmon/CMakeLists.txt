include (SDPAFindBoost)
find_package(Threads REQUIRED QUIET)
find_package (Qt4 4.7.1 COMPONENTS QtCore QtGui QtNetwork REQUIRED QUIET)
find_package (UTIL REQUIRED QUIET)
find_package (GSPC_RIF REQUIRED QUIET)

include (SDPAFindBoost)
include (${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gspc/mon)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${UTIL_INCLUDE_DIR})

# used for boost_connect.hpp
include_directories (${CMAKE_SOURCE_DIR}/editor/src ${Boost_INCLUDE_DIR})

qt4_wrap_cpp (wrapped_cpp_client
  gspc/mon/pnete/ui/monitor_client.hpp
  gspc/mon/pnete/ui/node_state_widget.hpp
  OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})

set (gspcmon_client_src
  gspc/mon/pnete/ui/monitor_client.cpp
  gspc/mon/pnete/ui/node_state_widget.cpp
  ${wrapped_cpp_client}
)

add_library (gspcmon_client        STATIC ${gspcmon_client_src})

add_executable (gspcmonc bin/gspcmonc.cpp)
target_link_libraries (gspcmonc
  gspcmon_client
  ${QT_LIBRARIES} fhg_util_qt fhg-util
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

install (TARGETS gspcmonc
  DESTINATION bin
  COMPONENT runtime
)
