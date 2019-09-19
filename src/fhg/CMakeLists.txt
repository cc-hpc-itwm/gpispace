configure_file (${CMAKE_CURRENT_SOURCE_DIR}/revision.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new @ONLY)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new
  RESULT_VARIABLE revision_cpp_needs_update
  OUTPUT_QUIET
  ERROR_QUIET
)
if (revision_cpp_needs_update)
  file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp)
  file (WRITE "${CMAKE_BINARY_DIR}/revision" "${PROJECT_REVISION}")
endif()

extended_add_library (NAME fhg-revision
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/revision.cpp"
)

extended_add_library (NAME fhg-util
  SOURCES "util/backtracing_exception.cpp"
          "util/signal_handler_manager.cpp"
          "util/remove_prefix.cpp"
          "util/read_bool.cpp"
          "util/now.cpp"

          "util/parse/position.cpp"
          "util/parse/error.cpp"
          "util/parse/require.cpp"
          "util/parse/until.cpp"

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
  LIBRARIES Util::Generic
            fhglog
            Boost::thread
            Boost::system
            Boost::filesystem
  INSTALL
)

add_subdirectory (util/test)

install (FILES
  util/parse/position.hpp
  util/parse/error.hpp
  util/parse/require.hpp
  DESTINATION include/fhg/util/parse
)

install (FILES
  util/dl.hpp
  DESTINATION include/fhg/util
)

install (FILES "${CMAKE_BINARY_DIR}/revision" DESTINATION .)
