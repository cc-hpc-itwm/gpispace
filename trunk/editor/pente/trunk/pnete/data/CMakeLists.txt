include(${QT_USE_FILE})

set(data_SRCS
  change_manager.cpp
  internal.cpp
  manager.cpp
  proxy.cpp
)

set(data_HDRS
  change_manager.hpp
  internal.hpp
  manager.hpp
  proxy.hpp
)

set(data_UIS
)
set(data_TOMOC
  change_manager.hpp
)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(data_UIS_H ${data_UIS})
QT4_WRAP_CPP(data_SRCS_MOC ${data_TOMOC} ${data_UIS_H})

qt4_automoc(${data_SRCS_MOC})
add_library (data ${data_SRCS} ${data_SRCS_MOC})
