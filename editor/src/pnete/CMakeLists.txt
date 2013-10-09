# bernd.loerwald@itwm.fraunhofer.de

set (base_sources
  data/change_manager.cpp
  data/internal.cpp
  data/manager.cpp
  data/handle/base.cpp
  data/handle/connect.cpp
  data/handle/expression.cpp
  data/handle/function.cpp
  data/handle/module.cpp
  data/handle/net.cpp
  data/handle/place.cpp
  data/handle/place_map.cpp
  data/handle/port.cpp
  data/handle/transition.cpp
)

set (base_tomoc
  data/change_manager.hpp
)


set (base_ui_sources
  ui/dock_widget.cpp
  ui/document_view.cpp
  ui/editor_window.cpp
)

set (base_ui_tomoc
  ui/dock_widget.hpp
  ui/document_view.hpp
  ui/editor_window.hpp
)


set (editor_ui_sources
  ui/ComboItemDelegate.cpp
  ui/StructureView.cpp
  ui/TransitionLibraryItem.cpp
  ui/TransitionLibraryModel.cpp
  ui/expression_widget.cpp
  ui/graph_view.cpp
  ui/module_call_widget.cpp
  ui/net_widget.cpp
  ui/port_lists_widget.cpp
  ui/size.cpp
  ui/transition_library_view.cpp
  ui/graph/association.cpp
  ui/graph/base_item.cpp
  ui/graph/cogwheel_button.cpp
  ui/graph/connectable_item.cpp
  ui/graph/connection.cpp
  ui/graph/orientation.cpp
  ui/graph/pending_connection.cpp
  ui/graph/place.cpp
  ui/graph/place_map.cpp
  ui/graph/popover_widget.cpp
  ui/graph/port.cpp
  ui/graph/port_place_association.cpp
  ui/graph/scene.cpp
  ui/graph/transition.cpp
  ui/graph/style/association.cpp
  ui/graph/style/cap.cpp
  ui/graph/style/fallback.cpp
  ui/graph/style/predicate.cpp
  ui/graph/style/raster.cpp
  ui/graph/style/size.cpp
  ui/graph/style/type.cpp
  weaver/display.cpp
  weaver/tv.cpp
)

set (editor_ui_uis
  ui/PropertiesEditorForm.ui
  ui/StructureEditorForm.ui
)

set (editor_ui_tomoc
  ui/StructureView.hpp
  ui/StructureEditor.hpp
  ui/PropertiesEditor.hpp
  ui/TransitionLibraryItem.hpp
  ui/TransitionLibraryModel.hpp
  ui/execution_monitor.hpp
  ui/expression_widget.hpp
  ui/graph/association.hpp
  ui/graph/base_item.hpp
  ui/graph/connectable_item.hpp
  ui/graph/connection.hpp
  ui/graph/place.hpp
  ui/graph/place_map.hpp
  ui/graph/popover_widget.hpp
  ui/graph/port.hpp
  ui/graph/port_place_association.hpp
  ui/graph/scene.hpp
  ui/graph/transition.hpp
  ui/graph_view.hpp
  ui/log_monitor.hpp
  ui/module_call_widget.hpp
  ui/net_widget.hpp
  ui/port_lists_widget.hpp
)

set (editor_ui_resources
  resources/resources.qrc
)


set (monitor_ui_sources
  ui/execution_monitor.cpp
  ui/execution_monitor_detail.cpp
  ui/execution_monitor_worker_model.cpp
  ui/gspc_monitor.cpp
  ui/gspc_monitor_client.cpp
  ui/gspc_monitor_detail.cpp
  ui/log_monitor.cpp
)
set (monitor_ui_tomoc
  ui/execution_monitor.hpp
  ui/execution_monitor_detail.hpp
  ui/execution_monitor_worker_model.hpp
  ui/gspc_monitor.hpp
  ui/gspc_monitor_client.hpp
  ui/gspc_monitor_detail.hpp
  ui/log_monitor.hpp
)


if (ENABLE_PNET_EDITOR)
  qt4_wrap_cpp (base_wrapped_cpps ${base_tomoc} OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})
  qt4_wrap_cpp (base_ui_wrapped_cpps ${base_ui_tomoc} OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})
  qt4_wrap_cpp (editor_ui_wrapped_cpps ${editor_ui_tomoc} OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})
  qt4_wrap_ui (editor_ui_wrapped_uis ${editor_ui_uis})
  qt4_add_resources (editor_ui_resource_cpps ${editor_ui_resources})
  qt4_wrap_cpp (monitor_ui_wrapped_cpps ${monitor_ui_tomoc} OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})

  add_executable (pnete
    pnete.cpp setting.cpp ${base_sources} ${base_wrapped_cpps}
    ${base_ui_sources} ${base_ui_wrapped_cpps} ${editor_ui_sources}
    ${editor_ui_wrapped_cpps} ${editor_ui_wrapped_uis} ${editor_ui_resource_cpps}
    ${monitor_ui_sources} ${monitor_ui_wrapped_cpps})

  target_link_libraries ( pnete
                          "${LIB_PREFIX}graphviz-wrapper"
                          "fhg_util_qt"
                          pnet
                          fhg-util
                          ${FhgLog_LIBRARY}
                          ${UTIL_LIBRARIES}
                          ${Boost_LIBRARIES}
                          ${QT_LIBRARIES}
                          fhg-plugin)

  install (TARGETS pnete RUNTIME DESTINATION bin COMPONENT runtime)

  add_subdirectory (ui/graph/style/test)
endif()

if (ENABLE_SDPA_GUI)
  qt4_wrap_cpp (monitor_ui_wrapped_cpps ${monitor_ui_tomoc}
    OPTIONS ${FHG_QT_MOC_BOOST_HACK_OPTIONS})

  add_executable ( sdpa-gui
    monitor.cpp ${monitor_ui_sources} ${monitor_ui_wrapped_cpps})

  add_executable (gspcmonc
    gspcmonc.cpp ${monitor_ui_sources} ${monitor_ui_wrapped_cpps})

  target_link_libraries ( sdpa-gui
                          pnet
                          "fhg_util_qt"
                          fhg-revision
                          ${FhgLog_LIBRARY}
                          ${Boost_LIBRARIES}
                          ${QT_LIBRARIES})

  install (TARGETS sdpa-gui RUNTIME DESTINATION bin COMPONENT runtime)

  target_link_libraries ( gspcmonc
                          pnet
                          "fhg_util_qt"
                          fhg-util
                          ${FhgLog_LIBRARY}
                          ${Boost_LIBRARIES}
                          ${QT_LIBRARIES})

  install (TARGETS sdpa-gui RUNTIME DESTINATION bin COMPONENT runtime)

  add_subdirectory (test)
endif()
