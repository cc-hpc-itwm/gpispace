set(task-view_SRC
  taskscene.cpp
  taskview.cpp
)

set(task-view_UI
  )

set(task-view_HDR
  taskscene.hpp
  taskview.hpp
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
QT4_WRAP_UI(task-view_UI_H ${task-view_UI})
QT4_WRAP_CPP(task-view_MOC_SRC ${task-view_HDR} ${task-view_UI_H})

qt4_automoc(${task-view_MOC_SRC})
add_library(task-view ${task-view_SRC} ${task-view_MOC_SRC})

install(TARGETS task-view
  ARCHIVE
  DESTINATION lib
  COMPONENT staticlibraries
  )
