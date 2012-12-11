# bernd.loerwald@itwm.fraunhofer.de

set (fhg_util_qt_headers
  cast.hpp
  parent.hpp
  scoped_signal_block.hpp
  boost_connect.hpp
  boost_connect_impl.hpp
  boost_connect_specializes.hpp
)

set (fhg_util_qt_sources
  boost_connect_impl.cpp
)

add_library (fhg_util_qt ${fhg_util_qt_sources})
target_link_libraries (fhg_util_qt ${QT_LIBRARIES})
