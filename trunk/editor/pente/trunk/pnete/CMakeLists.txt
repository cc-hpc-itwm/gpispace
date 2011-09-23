# -*- mode: cmake; -*-
set(QT_USE_QTXML on)
set(QT_USE_QTXMLPATTERNS on)
include(${QT_USE_FILE})
include(SDPAFindBoost)

find_package (XML REQUIRED)
find_package (WE REQUIRED)
find_package (UTIL REQUIRED)
find_package (FhgLog REQUIRED)
find_package (Graphviz REQUIRED)

set(pnete_SRCS
  pnete.cpp
)

set(pnete_RCCS
  resources/resources.qrc
)

QT4_ADD_RESOURCES(pnete_RCC_SRCS ${pnete_RCCS})

add_executable(pnete ${pnete_SRCS} ${pnete_RCC_SRCS})
add_dependencies(pnete gen-includes)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${XML_INCLUDE_DIR}
                    ${WE_INCLUDE_DIR}
                    ${UTIL_INCLUDE_DIR}
                    ${FhgLog_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIR}
                    ${GRAPHVIZ_INCLUDE_DIR}
                   )

add_subdirectory(data)
add_subdirectory(ui)
add_subdirectory(weaver)

target_link_libraries (pnete
  ui
  data
  weaver
  ${FhgLog_LIBRARY}
  ${Boost_LIBRARIES}
  ${QT_LIBRARIES}
  ${TIFF_LIBRARY}
  ${JPEG_LIBRARY}
  ${MNG_LIBRARY}
  ${GRAPHVIZ_LIBRARIES}
#  gobject-2.0
)

install(TARGETS pnete
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
