# -*- mode: cmake; -*-
include (${QT_USE_FILE})

set ( pnete_SRCS
      pnete.cpp
    )

set ( pnete_RCCS
      resources/resources.qrc
    )

QT4_ADD_RESOURCES (pnete_RCC_SRCS ${pnete_RCCS})

add_executable (pnete ${pnete_SRCS} ${pnete_RCC_SRCS})
add_dependencies(pnete gen_headerlist)

add_subdirectory (data)
add_subdirectory (ui)
add_subdirectory (weaver)

target_link_libraries ( pnete
                        "${LIB_PREFIX}ui"
                        "${LIB_PREFIX}data"
                        "${LIB_PREFIX}weaver"
                        ${WE_LIBRARIES}
                        ${FhgLog_LIBRARY}
                        ${Boost_LIBRARIES}
                        ${QT_LIBRARIES}
#                        ${TIFF_LIBRARY}
#                        ${JPEG_LIBRARY}
#                        ${MNG_LIBRARY}
                      )

install ( TARGETS pnete
          RUNTIME
          DESTINATION bin
          COMPONENT applications
        )
