# bernd.loerwald@itwm.fraunhofer.de

set ( pnete_SRCS
      pnete.cpp
      setting.cpp
    )

set ( pnete_RCCS
      resources/resources.qrc
    )

QT4_ADD_RESOURCES (pnete_RCC_SRCS ${pnete_RCCS})

add_executable (pnete ${pnete_SRCS} ${pnete_RCC_SRCS})

add_subdirectory (data)
add_subdirectory (ui)
add_subdirectory (weaver)

target_link_libraries ( pnete
                        "${LIB_PREFIX}ui"
                        "${LIB_PREFIX}data"
                        "${LIB_PREFIX}weaver"
                        we
                        ${FhgLog_LIBRARY}
                        ${UTIL_LIBRARIES}
                        ${Boost_LIBRARIES}
                        ${QT_LIBRARIES}
                        xml
                      )

install ( TARGETS pnete
          RUNTIME
          DESTINATION bin
          COMPONENT runtime
        )
