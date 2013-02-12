# bernd.loerwald@itwm.fraunhofer.de

include (${QT_USE_FILE})

set (sdpa-gui_SRCS
  main.cpp
  monitorwindow.cpp
  task.cpp)

set (sdpa-gui_TOMOC
  monitorwindow.hpp)

include_directories (${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

qt4_wrap_cpp (sdpa-gui_SRCS_MOC ${sdpa-gui_TOMOC})
qt4_automoc (${sdpa-gui_SRCS_MOC})

add_executable (sdpa-gui ${sdpa-gui_SRCS} ${sdpa-gui_SRCS_MOC})
target_link_libraries (sdpa-gui
  we fhg-revision ${FhgLog_LIBRARY} ${Boost_LIBRARIES} ${QT_LIBRARIES})

install (TARGETS sdpa-gui RUNTIME DESTINATION bin COMPONENT runtime)
