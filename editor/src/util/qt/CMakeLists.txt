# bernd.loerwald@itwm.fraunhofer.de

qt4_wrap_cpp (moc_undoredo no_undoredo_lineedit.hpp OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})
add_library (fhg_util_qt boost_connect_impl.cpp no_undoredo_lineedit.cpp ${moc_undoredo})
target_link_libraries (fhg_util_qt ${QT_LIBRARIES})

add_subdirectory (test)
