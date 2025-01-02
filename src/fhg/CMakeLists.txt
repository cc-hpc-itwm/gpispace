string(TIMESTAMP COPYRIGHT_YEAR "%Y")
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/project_configuration.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/project_configuration.cpp"
  @ONLY
)

file (WRITE "${CMAKE_BINARY_DIR}/gspc_version" "${PROJECT_VERSION}")

gspc_add_library (NAME fhg-project_configuration
  TYPE STATIC
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/project_configuration.cpp"
)

add_library (fhg_assert INTERFACE)
target_link_libraries (fhg_assert INTERFACE fmt)

gspc_add_library (NAME fhg-util
  TYPE STATIC
  SOURCES "util/backtracing_exception.cpp"
          "util/read_bool.cpp"
          "util/remove_prefix.cpp"
          "util/signal_handler_manager.cpp"
          "util/starts_with.cpp"

          "util/parse/position.cpp"
          "util/parse/error.cpp"
          "util/parse/require.cpp"

          "util/cctype.cpp"
          "util/num.cpp"
          "util/num/show.cpp"
          "util/indenter.cpp"
          "util/cpp/block.cpp"
          "util/cpp/include.cpp"
          "util/cpp/include_guard.cpp"
          "util/cpp/namespace.cpp"
          "util/cpp/struct.cpp"

          "util/xml.cpp"

          "util/system_with_blocked_SIGCHLD.cpp"
  LIBRARIES gspc::logging
            Util::Generic
            Boost::thread
            Boost::system
            Boost::filesystem
            fhg_assert
)

add_subdirectory (util/test)

install (FILES "${CMAKE_BINARY_DIR}/gspc_version" DESTINATION .)
