#bernd.loerwald@itwm.fraunhofer.de

set(ui_SRCS
  editor_window.cpp
  graph_view.cpp
  TransitionLibraryModel.cpp
  TransitionLibraryItem.cpp
  StructureView.cpp
# undo_group_manager.cpp
  dock_widget.cpp
  document_view.cpp
  expression_view.cpp
  mod_view.cpp
  net_view.cpp
  view_manager.cpp
  module_call_widget.cpp
  expression_widget.cpp
  port_lists_widget.cpp
  ComboItemDelegate.cpp
  size.cpp
  transition_library_view.cpp

  expression_view.cpp
  mod_view.cpp
  net_view.cpp
)

set(ui_HDRS
  editor_window.hpp
  graph_view.hpp
  TransitionLibraryModel.hpp
  TransitionLibraryItem.hpp
  StructureView.hpp
# undo_group_manager.hpp

  dock_widget.hpp
  document_view.hpp
  expression_view.hpp
  mod_view.hpp
  net_view.hpp
  view_manager.hpp

  ComboItemDelegate.hpp
  PropertiesEditor.hpp
  StructureEditor.hpp
  module_call_widget.hpp
  expression_widget.hpp
  port_lists_widget.hpp
  size.hpp
  transition_library_view.hpp

  expression_view.hpp
  mod_view.hpp
  net_view.hpp
)

set(ui_UIS
     PropertiesEditorForm.ui
     StructureEditorForm.ui
)

set(ui_TOMOC
  editor_window.hpp
  graph_view.hpp
  TransitionLibraryModel.hpp
  TransitionLibraryItem.hpp

  dock_widget.hpp
  document_view.hpp
  expression_view.hpp
  mod_view.hpp
  net_view.hpp
  view_manager.hpp

  module_call_widget.hpp
  expression_widget.hpp
  port_lists_widget.hpp

  expression_view.hpp
  mod_view.hpp
  net_view.hpp

  StructureView.hpp
)

add_subdirectory (graph)
add_subdirectory (util)

QT4_WRAP_UI(ui_UIS_H ${ui_UIS})
QT4_WRAP_CPP(ui_SRCS_MOC ${ui_TOMOC} ${ui_UIS_H})
qt4_automoc(${ui_SRCS_MOC})

add_library("${LIB_PREFIX}ui" ${ui_SRCS} ${ui_SRCS_MOC})
target_link_libraries ("${LIB_PREFIX}ui" "${LIB_PREFIX}graph")
