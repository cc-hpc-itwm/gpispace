extended_add_library (NAME fhg-util
  NAMESPACE IML
  SOURCES "num.cpp"

          "parse/position.cpp"
          "parse/error.cpp"
          "parse/require.cpp"
          "parse/until.cpp"

          "system_with_blocked_SIGCHLD.cpp"
          "signal_handler_manager.cpp"

  LIBRARIES Util::Generic
            Boost::thread
            Boost::system
            Boost::filesystem
  INSTALL
)
