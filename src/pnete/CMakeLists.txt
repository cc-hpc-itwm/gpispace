fhg_add_runtime_executable (NAME gspc-monitor
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
            fhg-revision
            fhg-util
            fhg_util_qt
            gspc::logging
  CREATE_BUNDLE_INFO
)

fhg_add_runtime_executable (NAME gspcmonc
  SOURCES "gspcmonc.cpp"
          "ui/gspc_monitor.cpp"
          "ui/gspc_monitor_client.cpp"
          "ui/gspc_monitor_detail.cpp"
  MOC "ui/gspc_monitor.hpp"
      "ui/gspc_monitor_client.hpp"
      "ui/gspc_monitor_detail.hpp"
  LIBRARIES Qt5::Core
            Qt5::Gui
            Qt5::Widgets
            Qt5::Network
            Util::Qt
            fhg-util
            fhg_util_qt
  CREATE_BUNDLE_INFO
)

add_subdirectory (test)
