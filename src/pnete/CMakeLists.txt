fhg_add_runtime_executable (NAME sdpa-gui
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
            Qt4::QtCore
            Qt4::QtGui
            Util::Qt
            fhg-revision
            fhg-util
            fhg_util_qt
            fhglog
)

fhg_add_runtime_executable (NAME gspcmonc
  SOURCES "gspcmonc.cpp"
          "ui/gspc_monitor.cpp"
          "ui/gspc_monitor_client.cpp"
          "ui/gspc_monitor_detail.cpp"
  MOC "ui/gspc_monitor.hpp"
      "ui/gspc_monitor_client.hpp"
      "ui/gspc_monitor_detail.hpp"
  LIBRARIES Qt4::QtCore
            Qt4::QtGui
            Qt4::QtNetwork
            Util::Qt
            fhg-util
            fhg_util_qt
)

add_subdirectory (test)
