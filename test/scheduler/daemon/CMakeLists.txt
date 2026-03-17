gspc_add_test (NAME scheduler_daemon_NotificationEvent
  SOURCES NotificationEvent.cpp
  USE_BOOST
  LIBRARIES gspc_scheduler
            gspc_logging
            gspc_logging_test
            gspc_testing
)
