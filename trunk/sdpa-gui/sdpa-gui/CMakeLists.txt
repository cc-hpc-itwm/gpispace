include(${QT_USE_FILE})

set(sdpa-gui_SRCS
  main.cpp
  monitorwindow.cpp
  logeventwrapper.cpp
  portfolioeval.cpp
)

set(sdpa-gui_UIS
  monitorwindow.ui
  )

set(sdpa-gui_HDRS
  monitorwindow.hpp
  logeventwrapper.hpp
  windowappender.hpp
#  parameters.hpp
  portfolioeval.hpp
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
QT4_WRAP_UI(sdpa-gui_UIS_H ${sdpa-gui_UIS})
QT4_WRAP_CPP(sdpa-gui_SRCS_MOC ${sdpa-gui_HDRS} ${sdpa-gui_UIS_H})

qt4_automoc(${sdpa-gui_SRCS_MOC})
add_executable (sdpa-gui ${sdpa-gui_SRCS} ${sdpa-gui_SRCS_MOC})
target_link_libraries (sdpa-gui
        ${WE_LIBRARIES}
	sdpa-client
	sdpa
	sdpa-events
	sdpa-util
	sdpa-com
	${FhgLog_LIBRARY}
	${Boost_LIBRARIES}
	${QT_LIBRARIES}
)

install(FILES ${sdpa-gui_HDRS}
  DESTINATION include/sdpa-gui/sdpa-gui
  COMPONENT headers
  )

install(TARGETS sdpa-gui
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
