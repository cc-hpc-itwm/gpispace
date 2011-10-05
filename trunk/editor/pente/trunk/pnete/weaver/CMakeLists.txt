include(${QT_USE_FILE})

set(weaver_SRCS
  tv.cpp
  display.cpp
)

set(weaver_HDRS
  weaver.hpp
  tv.hpp
  display.hpp
)

set(weaver_UIS
)
set(weaver_TOMOC
)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(weaver_UIS_H ${weaver_UIS})
QT4_WRAP_CPP(weaver_SRCS_MOC ${weaver_TOMOC} ${weaver_UIS_H})

qt4_automoc(${weaver_SRCS_MOC})
add_library (weaver ${weaver_SRCS} ${weaver_SRCS_MOC})
add_dependencies(weaver gen_headerlist)
