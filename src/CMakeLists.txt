# bernd.loerwald@itwm.fraunhofer.de

if (ENABLE_PNET_EDITOR)
  find_package (Graphviz COMPONENTS cdt gvc graph pathplan REQUIRED QUIET)
  find_package (Qt4 4.8 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED QUIET)

  include_directories ( ${CMAKE_CURRENT_BINARY_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}
                        ${FhgLog_INCLUDE_DIR}
                        ${UTIL_INCLUDE_DIR}
                        ${CMAKE_SOURCE_DIR}/sdpa
                        ${CMAKE_SOURCE_DIR}/gspc
                        ${CMAKE_SOURCE_DIR}/pnet/src
                        ${FhgKernel_INCLUDE_DIR}
                        ${CMAKE_SOURCE_DIR} # to have sdpa plugins in sdpa/plugins, not plugins/
                      )
  include_directories (SYSTEM ${GRAPHVIZ_INCLUDE_DIR})

  set (LIB_PREFIX "fhg_pnete_")

  include(${QT_USE_FILE})

  add_subdirectory (pnete)
  add_subdirectory (util)
endif()
