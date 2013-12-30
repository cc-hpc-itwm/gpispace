add_subdirectory(util)

execute_process(COMMAND cat ${CMAKE_BINARY_DIR}/build.tstamp
                OUTPUT_VARIABLE FHG_BUILD_TIMESTAMP
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )

if ("${FHG_BUILD_TIMESTAMP}" STREQUAL "")
  execute_process (COMMAND "date" "+%y-%m" OUTPUT_VARIABLE __year_and_month OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(FHG_BUILD_TIMESTAMP "${__year_and_month}")
  set(__year_and_month)
endif ()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/revision.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new @ONLY)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new
  RESULT_VARIABLE revision_cpp_needs_update
  OUTPUT_QUIET
  ERROR_QUIET
)
if (revision_cpp_needs_update)
  file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp)
endif()

add_library (fhg-revision ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp)

install (FILES revision.hpp
               assert.hpp
               assert_helper.hpp
               assertion_failed.hpp
               error_codes.hpp
  DESTINATION include/fhg
  COMPONENT headers
  )

install (FILES
  util/parse/position.hpp
  util/parse/error.hpp
  util/parse/require.hpp
  DESTINATION include/fhg/util/parse
  COMPONENT sdk
)
