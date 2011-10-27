include(${QT_USE_FILE})

set(ui_SRCS
  editor_window.cpp
  GraphView.cpp
  TransitionLibraryModel.cpp
  TransitionLibraryItem.cpp
  StructureView.cpp
# undo_group_manager.cpp
  dock_widget.cpp
  base_editor_widget.cpp
  document_view.cpp
  expression_view.cpp
  mod_view.cpp
  net_view.cpp
  view_manager.cpp
  net_widget.cpp
  module_call_widget.cpp
  expression_widget.cpp
  port_list_widget.cpp
  port_lists_widget.cpp
  ComboItemDelegate.cpp
)

set(ui_HDRS
  editor_window.hpp
  GraphView.hpp
  TransitionLibraryModel.hpp
  TransitionLibraryItem.hpp
  StructureView.hpp
# undo_group_manager.hpp

  dock_widget.hpp
  base_editor_widget.hpp
  document_view.hpp
  expression_view.hpp
  mod_view.hpp
  net_view.hpp
  view_manager.hpp

  ComboItemDelegate.hpp
  PropertiesEditor.hpp
  StructureEditor.hpp
  net_widget.hpp
  module_call_widget.hpp
  expression_widget.hpp
  port_list_widget.hpp
  port_lists_widget.hpp
)

set(ui_UIS
#    PropertiesWidget.ui
     PropertiesEditorForm.ui
     StructureEditorForm.ui
)

set(ui_TOMOC
  editor_window.hpp
  GraphView.hpp
  TransitionLibraryModel.hpp
  TransitionLibraryItem.hpp
#  StructureView.hpp
#  PropertiesEditor.hpp
#  StructureEditor.hpp
# undo_group_manager.hpp

  dock_widget.hpp
  base_editor_widget.hpp
  document_view.hpp
  expression_view.hpp
  mod_view.hpp
  net_view.hpp
  view_manager.hpp

  net_widget.hpp
  module_call_widget.hpp
  expression_widget.hpp
  port_list_widget.hpp
  port_lists_widget.hpp
)

add_subdirectory (graph)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(ui_UIS_H ${ui_UIS})
QT4_WRAP_CPP(ui_SRCS_MOC ${ui_TOMOC} ${ui_UIS_H})

qt4_automoc(${ui_SRCS_MOC})
add_library("${LIB_PREFIX}ui" ${ui_SRCS} ${ui_SRCS_MOC})
target_link_libraries ("${LIB_PREFIX}ui" "${LIB_PREFIX}graph")
