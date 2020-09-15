extended_add_library (NAME fhg-util
  NAMESPACE IML
  SOURCES "num.cpp"
          "parse/error.cpp"
          "parse/position.cpp"
          "parse/require.cpp"
          "parse/until.cpp"

          "system_with_blocked_SIGCHLD.cpp"

  LIBRARIES Util::Generic
            Boost::base
)
