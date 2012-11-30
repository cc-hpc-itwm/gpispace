#bernd.loerwald@itwm.fraunhofer.de

set(ui_util_SRCS
  action.cpp
)

set(ui_util_HDRS
  action.hpp
)

set(ui_util_TOMOC
  action.hpp
)

QT4_WRAP_CPP(ui_util_SRCS_MOC ${ui_util_TOMOC} ${ui_util_UI_UTILS_H})
qt4_automoc(${ui_util_SRCS_MOC})

add_library("${LIB_PREFIX}ui_util" ${ui_util_SRCS} ${ui_util_SRCS_MOC})
target_link_libraries ("${LIB_PREFIX}ui_util")
