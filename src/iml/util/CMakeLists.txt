extended_add_library (NAME fhg-util
  NAMESPACE IML
  SOURCES "system_with_blocked_SIGCHLD.cpp"
  LIBRARIES Util::Generic
            Boost::base
)
