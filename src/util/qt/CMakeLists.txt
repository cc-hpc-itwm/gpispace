# bernd.loerwald@itwm.fraunhofer.de

qt4_wrap_cpp (moc_util_qt
  dual_list_selector.hpp
  file_line_edit.hpp
  mini_button.hpp
  mvc/alphanum_sort_proxy.hpp
  mvc/delegating_header_view.hpp
  mvc/filter_ignoring_branch_nodes_proxy.hpp
  mvc/fixed_proxy_models.hpp
  mvc/flat_to_tree_proxy.hpp
  mvc/multiplicated_column_proxy.hpp
  no_undoredo_lineedit.hpp
  restricted_tree_column_sorter.hpp
  treeview_with_delete.hpp
  OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})

add_library (fhg_util_qt
  boost_connect_impl.cpp
  dual_list_selector.cpp
  file_line_edit.cpp
  mini_button.cpp
  mvc/alphanum_sort_proxy.cpp
  mvc/delegating_header_view.cpp
  mvc/filter_ignoring_branch_nodes_proxy.cpp
  mvc/fixed_proxy_models.cpp
  mvc/flat_to_tree_proxy.cpp
  mvc/multiplicated_column_proxy.cpp
  mvc/section_index.cpp
  no_undoredo_lineedit.cpp
  restricted_tree_column_sorter.cpp
  scoped_disconnect.cpp
  treeview_with_delete.cpp
  ${moc_util_qt})

target_link_libraries (fhg_util_qt ${QT_LIBRARIES})

add_subdirectory (test)
