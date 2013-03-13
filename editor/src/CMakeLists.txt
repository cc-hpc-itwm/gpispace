# bernd.loerwald@itwm.fraunhofer.de

cmake_minimum_required (VERSION "2.6" FATAL_ERROR)

project (pnete)
string (TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

set (PROJECT_DESCRIPTION "Petri Net Editor")
set (PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set (PROJECT_COPYRIGHT   "Copyright 2011 Fraunhofer ITWM")
set (PROJECT_CONTACT     "Bernd Lörwald <bernd.loerwald@itwm.fraunhofer.de>")

include (SDPAFindBoost)
find_package (FhgLog REQUIRED QUIET)
find_package (Graphviz COMPONENTS cdt gvc graph pathplan REQUIRED QUIET)
find_package (Qt4 4.7.1 COMPONENTS QtCore QtGui QtXml REQUIRED QUIET)
find_package (UTIL REQUIRED QUIET)
find_package (FhgKernel REQUIRED QUIET)
find_package (SDPA REQUIRED QUIET)

include_directories ( ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_SOURCE_DIR}
                      ${FhgLog_INCLUDE_DIR}
                      ${UTIL_INCLUDE_DIR}
                      ${CMAKE_SOURCE_DIR}/sdpa
                      ${CMAKE_SOURCE_DIR}/we
                      ${CMAKE_SOURCE_DIR}/xml
                      ${FhgKernel_INCLUDE_DIR}
                      ${CMAKE_SOURCE_DIR} # to have sdpa plugins in sdpa/plugins, not plugins/
                    )
include_directories (SYSTEM
                    ${Boost_INCLUDE_DIR}
                    ${GRAPHVIZ_INCLUDE_DIR}
                    )

set (LIB_PREFIX "fhg_pnete_")

include(${QT_USE_FILE})

add_subdirectory (pnete)
add_subdirectory (util)
