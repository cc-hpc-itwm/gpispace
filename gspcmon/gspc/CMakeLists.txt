# used for boost_connect.hpp
include_directories (${CMAKE_SOURCE_DIR}/editor/src ${Boost_INCLUDE_DIR})

qt4_wrap_cpp (wrapped_cpp_client mon/node_state_widget.hpp OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})

set (gspcmon_client_src
  mon/node_state_widget.cpp
  ${wrapped_cpp_client}
)

add_library (gspcmon_client        STATIC ${gspcmon_client_src})
