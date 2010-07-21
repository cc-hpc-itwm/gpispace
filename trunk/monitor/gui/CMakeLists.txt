include(${QT_USE_FILE})

set(sdpa-gui_SRCS
  main.cpp
  sdpaWnd.cpp
)

set(sdpa-gui_MOC_HDRS
  CostumerEvent.h
  sdpaWnd.h
  Universal.h
  UniversalQt.h
)
QT4_WRAP_CPP(sdpa-gui_MOC_SRCS ${sdpa-gui_MOC_HDRS})

qt4_automoc(${sdpa-gui_SRCS})
add_executable (sdpa-gui ${sdpa-gui_SRCS} ${sdpa-gui_MOC_SRCS})
target_link_libraries (sdpa-gui
	${FhgLog_LIBRARY}
	${Boost_LIBRARIES}
	${QT_LIBRARIES}
)
