find_package (Qt5 REQUIRED
  COMPONENTS
    Core
    Gui
    Widgets
)

gspc_add_library (NAME gspc_util_qt
  TYPE STATIC
  SOURCES
    "do_after_event_loop.cpp"
    "dual_list_selector.cpp"
    "make_drop_target.cpp"
    "mvc/alphanum_sort_proxy.cpp"
    "mvc/delegating_header_view.cpp"
    "mvc/filter_ignoring_branch_nodes_proxy.cpp"
    "mvc/fixed_proxy_models.cpp"
    "mvc/flat_to_tree_proxy.cpp"
    "mvc/section_index.cpp"
    "restricted_tree_column_sorter.cpp"
    "scoped_disconnect.cpp"
    "style.cpp"
    "treeview_with_delete.cpp"
    "widget/file_line_edit.cpp"
    "widget/mini_button.cpp"
  MOC
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/dual_list_selector.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/mvc/alphanum_sort_proxy.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/mvc/delegating_header_view.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/mvc/filter_ignoring_branch_nodes_proxy.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/mvc/fixed_proxy_models.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/mvc/flat_to_tree_proxy.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/restricted_tree_column_sorter.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/treeview_with_delete.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/widget/file_line_edit.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/util/qt/widget/mini_button.hpp"
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES
    PUBLIC Boost::serialization
    PUBLIC gspc_util
    PUBLIC Qt5::Core
    PUBLIC Qt5::Gui
    PUBLIC Qt5::Widgets
    PUBLIC fmt
    PUBLIC gspc_assert
  COMPILE_OPTIONS
    PRIVATE -Wno-deprecated-declarations
)
