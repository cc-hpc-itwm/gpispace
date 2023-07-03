gspc_add_executable (NAME gspc-monitor
  RUNTIME
  SOURCES "monitor.cpp"
          "ui/execution_monitor.cpp"
          "ui/execution_monitor_detail.cpp"
          "ui/execution_monitor_worker_model.cpp"
          "ui/log_monitor.cpp"
  MOC "ui/execution_monitor.hpp"
      "ui/execution_monitor_detail.hpp"
      "ui/execution_monitor_worker_model.hpp"
      "ui/log_monitor.hpp"
  LIBRARIES Boost::thread
            Boost::program_options
            Qt5::Core
            Qt5::Gui
            Qt5::Widgets
            Util::Qt
            fhg-project_configuration
            fhg-util
            fhg_util_qt
            gspc::logging
  CREATE_BUNDLE_INFO
)

target_compile_options (gspc-monitor
  PRIVATE -Wno-deprecated-declarations
          $<$<CXX_COMPILER_ID:GNU>:-Wno-stringop-overflow>
)

add_subdirectory (test)
