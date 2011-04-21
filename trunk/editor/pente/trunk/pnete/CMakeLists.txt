include(${QT_USE_FILE})

set(pnete_SRCS
  main.cpp
  mainwindow.cpp
)

set(pnete_UIS
  mainwindow.ui
)

set(pnete_HDRS
  Nodes.h
  mainwindow.h
)

# todo: dont moc root!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(pnete_UIS_H ${pnete_UIS})
QT4_WRAP_CPP(pnete_SRCS_MOC ${pnete_HDRS} ${pnete_UIS_H})

qt4_automoc(${pnete_SRCS_MOC})
add_executable(pnete ${pnete_SRCS} ${pnete_SRCS_MOC})

add_subdirectory(graph)
add_subdirectory(ui)

target_link_libraries (pnete
  graph
  ui
 
	${FhgLog_LIBRARY}
#	${Boost_LIBRARIES}
	${QT_LIBRARIES}
)

install(TARGETS pnete
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
