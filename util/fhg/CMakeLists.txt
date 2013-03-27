configure_file(assert_config.hpp.in assert_config.hpp @ONLY)

add_subdirectory(util)

execute_process(COMMAND hostname -f
                OUTPUT_VARIABLE FHG_BUILD_HOST
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND id -un
                OUTPUT_VARIABLE FHG_BUILD_USER
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND uname -p
                OUTPUT_VARIABLE FHG_BUILD_CPU
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND uname -s
                OUTPUT_VARIABLE FHG_BUILD_KERNEL
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND uname -v
                OUTPUT_VARIABLE FHG_BUILD_KERNEL_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )

execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version
                COMMAND head -n 1
                OUTPUT_VARIABLE FHG_BUILD_COMPILER
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )

execute_process(COMMAND cat ${CMAKE_BINARY_DIR}/build.counter
                OUTPUT_VARIABLE FHG_BUILD_COUNT
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
                )

execute_process(COMMAND cat ${CMAKE_BINARY_DIR}/build.tstamp
                OUTPUT_VARIABLE FHG_BUILD_TIMESTAMP
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )

if ("${FHG_BUILD_COUNT}" STREQUAL "")
  set(FHG_BUILD_COUNT "0")
endif ()
MATH(EXPR FHG_BUILD_COUNT "${FHG_BUILD_COUNT}+1")

if ("${FHG_BUILD_TIMESTAMP}" STREQUAL "")
  set(FHG_BUILD_TIMESTAMP "Sun Jan  1 12:00:00 CET 2012")
endif ()

set(FHG_BUILD_INFO "${FHG_BUILD_KERNEL} ${FHG_BUILD_CPU} Build #${FHG_BUILD_COUNT} at ${FHG_BUILD_TIMESTAMP} by ${FHG_BUILD_USER}@${FHG_BUILD_HOST}")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/revision.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new @ONLY)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in  ${CMAKE_CURRENT_BINARY_DIR}/version.hpp.new @ONLY)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new
  RESULT_VARIABLE revision_cpp_needs_update
  OUTPUT_QUIET
  ERROR_QUIET
)
if (revision_cpp_needs_update)
  file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp)
#  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/revision.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp @ONLY)
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_CURRENT_BINARY_DIR}/version.hpp ${CMAKE_CURRENT_BINARY_DIR}/version.hpp.new
  RESULT_VARIABLE version_hpp_needs_update
  OUTPUT_QUIET
  ERROR_QUIET
)
if (version_hpp_needs_update)
  file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/version.hpp.new ${CMAKE_CURRENT_BINARY_DIR}/version.hpp)
endif()

add_library (fhg-revision ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp)

#add_custom_target(increment-build-counter ALL
#  COMMAND echo "${FHG_BUILD_COUNT}" > "${CMAKE_BINARY_DIR}/build.counter"
#  COMMENT "Incrementing build counter to ${FHG_BUILD_COUNT}"
#  VERBATIM
#  )

install (FILES revision.hpp
               assert.hpp
               assert_modes.hpp
               assert_helper.hpp
               assertion_failed.hpp
               error_codes.hpp
               ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
  DESTINATION include/fhg
  COMPONENT headers
  )

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
  DESTINATION include/fhg
  COMPONENT runtime-headers
)
