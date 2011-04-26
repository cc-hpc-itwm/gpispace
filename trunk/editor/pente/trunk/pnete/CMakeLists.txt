set(QT_USE_QTXML on)
set(QT_USE_QTXMLPATTERNS on)
include(${QT_USE_FILE})

set(pnete_SRCS
  pnete.cpp
)

add_executable(pnete ${pnete_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(helper)
add_subdirectory(graph)
add_subdirectory(data)
add_subdirectory(ui)

target_link_libraries (pnete
  helper
  graph
  data
  ui
 
	${FhgLog_LIBRARY}
#	${Boost_LIBRARIES}
	${QT_LIBRARIES}
)

install(TARGETS pnete
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
