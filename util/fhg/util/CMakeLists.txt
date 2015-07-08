extended_add_library (NAME fhg-util
  SOURCES "backtracing_exception.cpp"
          "signal_handler_manager.cpp"
          "remove_prefix.cpp"
          "read_bool.cpp"
          "now.cpp"

          "parse/position.cpp"
          "parse/error.cpp"
          "parse/require.cpp"
          "parse/until.cpp"

          "num.cpp"
          "num/show.cpp"
          "indenter.cpp"
          "cpp/block.cpp"
          "cpp/include.cpp"
          "cpp/include_guard.cpp"
          "cpp/namespace.cpp"
          "cpp/struct.cpp"

          "xml.cpp"

          "system_with_blocked_SIGCHLD.cpp"
  LIBRARIES Util::Generic
            Boost::thread
            Boost::system
            Boost::filesystem
            fhglog
)

  add_subdirectory(test)
