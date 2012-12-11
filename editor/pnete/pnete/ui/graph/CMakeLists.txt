#bernd.loerwald@itwm.fraunhofer.de

set(graph_SRCS
  association.cpp
  base_item.cpp
  connectable_item.cpp
  orientation.cpp

  pending_connection.cpp

  connection.cpp
  place.cpp
  port.cpp
  port_place_association.cpp
  scene.cpp
  transition.cpp

  cogwheel_button.cpp
  popover_widget.cpp
)

set(graph_HDRS
  association.hpp
  base_item.hpp
  connectable_item.hpp
  orientation.hpp

  pending_connection.hpp

  connection.hpp
  place.hpp
  port.hpp
  port_place_association.hpp
  scene.hpp
  transition.hpp

  cogwheel_button.hpp
  popover_widget.hpp
)

set(graph_TOMOC
  association.hpp
  base_item.hpp
  connectable_item.hpp

  connection.hpp
  place.hpp
  port.hpp
  port_place_association.hpp
  scene.hpp
  transition.hpp

  popover_widget.hpp
)

add_subdirectory (style)

QT4_WRAP_CPP(graph_SRCS_MOC ${graph_TOMOC})
qt4_automoc(${graph_SRCS_MOC})

add_library("${LIB_PREFIX}graph" ${graph_SRCS} ${graph_SRCS_MOC})
target_link_libraries ("${LIB_PREFIX}graph"
                       "${LIB_PREFIX}graphviz-wrapper"
                       "${LIB_PREFIX}style"
                       "${LIB_PREFIX}ui_util"
                       "fhg_util_qt"
                       )
