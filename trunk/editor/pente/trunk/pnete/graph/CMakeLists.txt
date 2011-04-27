include(${QT_USE_FILE})

set(graph_SRCS
  Style.cpp
  Connection.cpp
  ConnectableItem.cpp
  Port.cpp
  Scene.cpp
  Transition.cpp
)

set(graph_HDRS
  Style.hpp
  Connection.hpp
  ConnectableItem.hpp
  Port.hpp
  Scene.hpp
  Transition.hpp
)

set(graph_UIS
)
set(graph_TOMOC
  Scene.hpp
  Transition.hpp
  Port.hpp
  ConnectableItem.hpp
)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(graph_UIS_H ${graph_UIS})
QT4_WRAP_CPP(graph_SRCS_MOC ${graph_TOMOC} ${graph_UIS_H})

qt4_automoc(${graph_SRCS_MOC})
add_library (graph ${graph_SRCS} ${graph_SRCS_MOC})
