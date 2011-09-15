include(${QT_USE_FILE})

set(ui_SRCS
  GraphStyle.cpp
  GraphConnection.cpp
  GraphConnectableItem.cpp
  GraphPort.cpp
  GraphScene.cpp
  GraphTransition.cpp
  editor_window.cpp
  GraphView.cpp
  TransitionLibraryModel.cpp
  TransitionLibraryItem.cpp
  PopoverWidget.cpp
  GraphTransitionCogWheelButton.cpp
  StructureView.cpp
# undo_group_manager.cpp

  dockable_graph_view.cpp
  view_manager.cpp
)

set(ui_HDRS
  GraphStyle.hpp
  GraphConnection.hpp
  GraphConnectableItem.hpp
  GraphPort.hpp
  GraphScene.hpp
  GraphTransition.hpp
  editor_window.hpp
  GraphView.hpp
  TransitionLibraryModel.hpp
  TransitionLibraryItem.hpp
  PopoverWidget.hpp
  GraphTransitionCogWheelButton.hpp
  StructureView.hpp
# undo_group_manager.hpp

  dockable_graph_view.hpp
  view_manager.hpp

  graph_item.hpp
  ComboItemDelegate.hpp
  PropertiesEditor.hpp
  StructureEditor.hpp
)

set(ui_UIS
#    PropertiesWidget.ui
     PropertiesEditorForm.ui
     StructureEditorForm.ui
)

set(ui_TOMOC
  GraphConnectableItem.hpp
  GraphPort.hpp
  GraphScene.hpp
  GraphTransition.hpp
  editor_window.hpp
  GraphView.hpp
  TransitionLibraryModel.hpp
  TransitionLibraryItem.hpp
  PopoverWidget.hpp
  StructureView.hpp
  PropertiesEditor.hpp
  StructureEditor.hpp
# undo_group_manager.hpp

  dockable_graph_view.hpp
  view_manager.hpp

  graph_item.hpp
)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(ui_UIS_H ${ui_UIS})
QT4_WRAP_CPP(ui_SRCS_MOC ${ui_TOMOC} ${ui_UIS_H})

qt4_automoc(${ui_SRCS_MOC})
add_library(ui ${ui_SRCS} ${ui_SRCS_MOC})
