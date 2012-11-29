#bernd.loerwald@itwm.fraunhofer.de

set(weaver_SRCS
  tv.cpp
  display.cpp
)

set(weaver_HDRS
  weaver.hpp
  tv.hpp
  display.hpp
)

add_library ("${LIB_PREFIX}weaver" ${weaver_SRCS} ${weaver_SRCS_MOC})
add_dependencies("${LIB_PREFIX}weaver" gen_headerlist)
