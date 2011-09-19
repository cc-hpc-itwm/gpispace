include(${QT_USE_FILE})

set(traverse_SRCS
  display.cpp
)

set(traverse_HDRS
  display.hpp
)

set(traverse_UIS
)

set(traverse_TOMOC
)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(traverse_UIS_H ${traverse_UIS})
QT4_WRAP_CPP(traverse_SRCS_MOC ${traverse_TOMOC} ${traverse_UIS_H})

qt4_automoc(${traverse_SRCS_MOC})
add_library(traverse ${traverse_SRCS} ${traverse_SRCS_MOC})
