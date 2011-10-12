include(${QT_USE_FILE})

set(style_SRCS
  port.cpp
  connection.cpp
  transition.cpp
  cap.cpp
)

set(style_HDRS
  cap.hpp
)

set(style_UIS
)

set(style_TOMOC
)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(style_UIS_H ${style_UIS})
QT4_WRAP_CPP(style_SRCS_MOC ${style_TOMOC} ${style_UIS_H})

qt4_automoc(${style_SRCS_MOC})
add_library(style ${style_SRCS} ${style_SRCS_MOC})
