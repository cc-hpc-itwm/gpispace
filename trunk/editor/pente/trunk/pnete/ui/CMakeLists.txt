include(${QT_USE_FILE})

set(ui_SRCS
  GraphView.cpp
)

set(ui_HDRS
  GraphView.hpp
)

set(ui_UIS
)
set(ui_TOMOC
  GraphView.hpp
)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(ui_UIS_H ${ui_UIS})
QT4_WRAP_CPP(ui_SRCS_MOC ${ui_TOMOC} ${ui_UIS_H})

qt4_automoc(${ui_SRCS_MOC})
add_library(ui ${ui_SRCS} ${ui_SRCS_MOC})
