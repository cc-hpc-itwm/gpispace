include(${QT_USE_FILE})

set(graph_SRCS
  style.cpp
  connection.cpp
  connectable_item.cpp
  port.cpp
  scene.cpp
  transition.cpp
  cogwheel_button.cpp
  place.cpp

  popover_widget.cpp
)

set(graph_HDRS
  style.hpp
  connection.hpp
  connectable_item.hpp
  port.hpp
  scene.hpp
  transition.hpp
  cogwheel_button.hpp
  item.hpp
  place.hpp

  popover_widget.hpp
)

set(graph_UIS
)

set(graph_TOMOC
  style.hpp
  connection.hpp
  connectable_item.hpp
  port.hpp
  scene.hpp
  transition.hpp
  cogwheel_button.hpp
  item.hpp
  place.hpp

  popover_widget.hpp
)


#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(graph_UIS_H ${graph_UIS})
QT4_WRAP_CPP(graph_SRCS_MOC ${graph_TOMOC} ${graph_UIS_H})

qt4_automoc(${graph_SRCS_MOC})
add_library(graph ${graph_SRCS} ${graph_SRCS_MOC})
