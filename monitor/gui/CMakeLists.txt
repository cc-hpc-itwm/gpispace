# -*- mode: cmake; -*-
include(${QT_USE_FILE})

set(sdpa-mon_SRCS
  main.cpp
  sdpaWnd.cpp
)

set(sdpa-mon_MOC_HDRS
  CostumerEvent.h
  sdpaWnd.h
)
QT4_WRAP_CPP(sdpa-mon_MOC_SRCS ${sdpa-mon_MOC_HDRS})

qt4_automoc(${sdpa-mon_SRCS})
add_executable (sdpa-mon ${sdpa-mon_SRCS} ${sdpa-mon_MOC_SRCS})
target_link_libraries (sdpa-mon
	${FhgLog_LIBRARY}
	${Boost_LIBRARIES}
	${QT_LIBRARIES}
#	gobject-2.0
#	${TIFF_LIBRARY}
#	${JPEG_LIBRARY}
#	${MNG_LIBRARY}
#	${LCMS_LIBRARY}
)

install(TARGETS sdpa-mon
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
