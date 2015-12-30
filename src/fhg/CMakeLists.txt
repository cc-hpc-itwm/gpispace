add_subdirectory(util)

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
