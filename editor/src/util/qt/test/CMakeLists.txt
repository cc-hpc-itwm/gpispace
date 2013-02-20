#bernd.loerwald@itwm.fraunhofer.de

set (files_to_moc boost_connect.hpp)

qt4_wrap_cpp (mocd_files ${files_to_moc})
qt4_automoc (${mocd_files})

fhg_add_test (boost_connect.cpp ${mocd_files}
  LINK_LIBRARIES fhg_util_qt ${QT_LIBRARIES}
  DESCRIPTION "connecting a Qt signal to a boost::function"
)
