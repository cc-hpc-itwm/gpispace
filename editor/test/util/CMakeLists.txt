#bernd.loerwald@itwm.fraunhofer.de

include (SDPAFindBoost)
find_package (Qt4 4.7.1 COMPONENTS QtCore QtGui QtXml REQUIRED QUIET)

include_directories ( ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_SOURCE_DIR}
                      ${CMAKE_SOURCE_DIR}/editor/pnete
                      ${Boost_INCLUDE_DIR}
                    )


include(${QT_USE_FILE})

set (files_to_moc boost_connect.hpp)

qt4_wrap_cpp (mocd_files ${files_to_moc})
qt4_automoc (${mocd_files})

fhg_add_test (boost_connect.cpp ${mocd_files}
  LINK_LIBRARIES fhg_util_qt ${QT_LIBRARIES}
  DESCRIPTION "connecting a Qt signal to a boost::function"
)
