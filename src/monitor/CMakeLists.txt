gspc_add_library (NAME gspc_monitor
  TYPE STATIC
  SOURCES "execution_monitor.cpp"
          "execution_monitor_detail.cpp"
          "execution_monitor_worker_model.cpp"
          "log_monitor.cpp"
  MOC "${PROJECT_SOURCE_DIR}/include/gspc/monitor/execution_monitor.hpp"
      "${PROJECT_SOURCE_DIR}/include/gspc/monitor/execution_monitor_detail.hpp"
      "${PROJECT_SOURCE_DIR}/include/gspc/monitor/execution_monitor_worker_model.hpp"
      "${PROJECT_SOURCE_DIR}/include/gspc/monitor/log_monitor.hpp"
  LIBRARIES Boost::thread
            Qt5::Core
            Qt5::Gui
            Qt5::Widgets
            gspc_util_qt
            gspc_configuration
            gspc_util
            gspc_logging
            gspc_scheduler
)

target_include_directories (gspc_monitor
  PUBLIC "${PROJECT_SOURCE_DIR}/include"
)

target_compile_options (gspc_monitor
  PRIVATE -Wno-deprecated-declarations
          $<$<CXX_COMPILER_ID:GNU>:-Wno-stringop-overflow>
)
