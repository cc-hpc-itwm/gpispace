###############################################################################
# Checks
###############################################################################

if (NOT BUILD_TESTING OR NOT ${_project_entry}_BUILD_TESTING)
  return ()
endif ()

###############################################################################
# Includes
###############################################################################

include (util-cmake/add_unit_test)

###############################################################################
# Definitions
###############################################################################

function (rpc_unit_test _path)
  _parse_arguments_with_unknown (ARG "" "" "LIBRARIES;SOURCES" "" ${ARGN})
  string (REPLACE "/" "-" _name "rpc-${_path}")

  add_unit_test (NAME "${_name}"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${_path}.cpp" ${ARG_SOURCES}
    INCLUDE_DIRECTORIES
      PRIVATE "${PROJECT_SOURCE_DIR}/test"
      PRIVATE "${PROJECT_SOURCE_DIR}/src"
    USE_BOOST
    LIBRARIES Util::RPC ${ARG_LIBRARIES}
    ${ARG_UNPARSED_ARGUMENTS}
  )
endfunction()

###############################################################################
# Test declarations
###############################################################################

rpc_unit_test (basic)

rpc_unit_test (exceptions)

rpc_unit_test (future)

rpc_unit_test (locked_with_info_file PRIVATE fmt)

rpc_unit_test (many_threads_hammer_one_server RUN_SERIAL)

rpc_unit_test (performance PERFORMANCE_TEST)
set_tests_properties (rpc-performance PROPERTIES TIMEOUT 300)

rpc_unit_test (socket)

rpc_unit_test (socket_operation_preconditions)

rpc_unit_test (templates)
