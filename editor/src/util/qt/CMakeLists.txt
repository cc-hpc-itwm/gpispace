# bernd.loerwald@itwm.fraunhofer.de

qt4_wrap_cpp (moc_util_qt
  file_line_edit.hpp
  no_undoredo_lineedit.hpp
  OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})

add_library (fhg_util_qt
  boost_connect_impl.cpp
  file_line_edit.cpp
  no_undoredo_lineedit.cpp
  ${moc_util_qt})
target_link_libraries (fhg_util_qt ${QT_LIBRARIES})

add_subdirectory (test)
