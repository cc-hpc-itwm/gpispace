# -*- mode: cmake; -*-

include(${QT_USE_FILE})

set(sdpa-gui_SRCS
  main.cpp
  monitorwindow.cpp
  logeventwrapper.cpp
  task.cpp
)

set(sdpa-gui_HDRS
  monitorwindow.hpp
  logeventwrapper.hpp
  windowappender.hpp
)

set (sdpa-gui_TOMOC
  monitorwindow.hpp
)

# FIXME: I get a linker error, when I try to link against sdpa-util (Bad value),
# which seems to be  a known compiler bug in gcc 4.1.2,  by explicitly using the
# source files, everything is fine but that just sucks.
set(util-sources
  ${CMAKE_SOURCE_DIR}/sdpa/sdpa/util/Config.cpp
  ${CMAKE_SOURCE_DIR}/sdpa/sdpa/util/util.cpp
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
QT4_WRAP_CPP(sdpa-gui_SRCS_MOC ${sdpa-gui_TOMOC})

qt4_automoc(${sdpa-gui_SRCS_MOC})
add_executable (sdpa-gui ${sdpa-gui_SRCS} ${sdpa-gui_SRCS_MOC} ${util-sources})
target_link_libraries (sdpa-gui
        we
	sdpa-client
	sdpa
	sdpa-events
#	sdpa-util
	sdpa-com
	fhg-revision
	seda
	${FhgLog_LIBRARY}
	${Boost_LIBRARIES}
	${QT_LIBRARIES}
#        gobject-2.0
#	${TIFF_LIBRARY}
#	${JPEG_LIBRARY}
#	${MNG_LIBRARY}
#	${LCMS_LIBRARY}
)

install(FILES ${sdpa-gui_HDRS}
  DESTINATION include/sdpa-gui/sdpa-gui
  COMPONENT headers
  )

install(TARGETS sdpa-gui
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
