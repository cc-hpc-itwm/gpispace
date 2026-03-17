###############################################################################
# Subdirectories
###############################################################################

add_subdirectory (qt)

###############################################################################
# Definitions
###############################################################################

function (gspc_util_unit_test _path)
  _parse_arguments_with_unknown (ARG
    "" "" "LIBRARIES;SOURCES;REQUIRE_FAIL" "" ${ARGN}
  )
  string (REPLACE "/" "_" _name "gspc_test_util_${_path}")

  if (NOT ARG_REQUIRE_FAIL)
    add_unit_test (NAME "${_name}"
      SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${_path}.cpp" ${ARG_SOURCES}
      USE_BOOST
      LIBRARIES gspc_util gspc_testing ${ARG_LIBRARIES} PRIVATE fmt
      ${ARG_UNPARSED_ARGUMENTS}
    )
  else()
    # \todo move REQUIRE_FAIL support one level down to not recompile
    # for every case.
    foreach (_case ${ARG_REQUIRE_FAIL})
      add_unit_test (NAME "${_name}-${_case}"
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${_path}.cpp" ${ARG_SOURCES}
        USE_BOOST
        LIBRARIES gspc_util gspc_testing ${ARG_LIBRARIES}
        ARGS --run_test="${_case}"
        ${ARG_UNPARSED_ARGUMENTS}
      )
      set_tests_properties ("${_name}-${_case}" PROPERTIES WILL_FAIL true)
    endforeach()
  endif()
endfunction()

###############################################################################
# Test declarations - util-generic tests
###############################################################################

gspc_util_unit_test (asynchronous)

gspc_util_unit_test (blocked)

gspc_util_unit_test (callable_signature)

gspc_util_unit_test (chrono_unit_for_period)

gspc_util_unit_test (connectable_to_address_string)

gspc_util_unit_test (cxx17/holds_alternative)

gspc_util_unit_test (cxx17/void_t)

gspc_util_unit_test (divru)

extended_add_library (NAME gspc-util-dynamic_linking-test_library
  TYPE SHARED
  SOURCES "dynamic_linking-lib.cpp"
  LIBRARIES gspc_util
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)
set_property (TARGET gspc-util-dynamic_linking-test_library
  PROPERTY CXX_VISIBILITY_PRESET hidden
)

gspc_util_unit_test (dynamic_linking
  DEPENDS gspc-util-dynamic_linking-test_library
  COMPILE_DEFINITIONS PRIVATE
    "-DGSPC_UTIL_TESTING_LIB_PATH=\"$<TARGET_FILE:gspc-util-dynamic_linking-test_library>\""
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

gspc_util_unit_test (executable_path)

gspc_util_unit_test (fallthrough)

gspc_util_unit_test (filesystem_lock_directory)

gspc_util_unit_test (finally)

gspc_util_unit_test (first_then)

gspc_util_unit_test (functor_visitor)

gspc_util_unit_test (hard_integral_typedef)

gspc_util_unit_test (hash/combined_hash)

gspc_util_unit_test (hash/combined_hash-performance PERFORMANCE_TEST)

gspc_util_unit_test (hostname)

gspc_util_unit_test (in_this_scope)

gspc_util_unit_test (join)

gspc_util_unit_test (latch RUN_SERIAL)

gspc_util_unit_test (make_optional)

gspc_util_unit_test (mp)

gspc_util_unit_test (ndebug)

gspc_util_unit_test (ostream/callback/bracket)

gspc_util_unit_test (ostream/callback/close)

gspc_util_unit_test (ostream/callback/function)

gspc_util_unit_test (ostream/callback/open)

gspc_util_unit_test (ostream/callback/print)

gspc_util_unit_test (ostream/callback/range)

gspc_util_unit_test (ostream/echo)

gspc_util_unit_test (ostream/line_by_line)

gspc_util_unit_test (ostream/modifier)

gspc_util_unit_test (ostream/prefix_per_line)

gspc_util_unit_test (ostream/put_time)

gspc_util_unit_test (ostream/redirect)

gspc_util_unit_test (ostream/redirect_standard_streams)

gspc_util_unit_test (ostream/to_string)

gspc_util_unit_test (print_container)

gspc_util_unit_test (print_exception)

gspc_util_unit_test (procfs
  ARGS --some args --to actually --test something, yeah
)

gspc_util_unit_test (boost/program_options/generic
  LIBRARIES Boost::program_options
)

gspc_util_unit_test (boost/program_options/separated_argument_list_parser
  LIBRARIES Boost::program_options
)

gspc_util_unit_test (read_file)

gspc_util_unit_test (read_lines)

gspc_util_unit_test (refcounted_set)

gspc_util_unit_test (scoped_boost_asio_io_service_with_threads
  LIBRARIES Boost::thread
)

gspc_util_unit_test (scoped_file_with_content)

gspc_util_unit_test (serialization/asio/ip/tcp/endpoint)

gspc_util_unit_test (serialization/asio/local/stream_protocol)

gspc_util_unit_test (serialization/boost/blank)

gspc_util_unit_test (serialization/by_member)

gspc_util_unit_test (serialization/exception)

gspc_util_unit_test (serialization/std/chrono)

gspc_util_unit_test (serialization/std/error_code)

gspc_util_unit_test (serialization/std/tuple)

gspc_util_unit_test (serialization/trivial)

gspc_util_unit_test (split)

extended_add_executable (NAME gspc_util_system_command
  SOURCES "system_command.cpp"
  LIBRARIES gspc_util
)

gspc_util_unit_test (system
  LIBRARIES PRIVATE fmt
  DEPENDS gspc_util_system_command.exe
  ARGS "$<TARGET_FILE:gspc_util_system_command.exe>"
)

gspc_util_unit_test (temporary_file)

gspc_util_unit_test (temporary_path)

# Testing framework tests moved to test/testing/

gspc_util_unit_test (this_bound_mem_fn)

gspc_util_unit_test (threadsafe_queue RUN_SERIAL)

gspc_util_unit_test (timer)

gspc_util_unit_test (va_args)

gspc_util_unit_test (wait_and_collect_exceptions)

gspc_util_unit_test (write_file)

###############################################################################
# Test declarations - fhg/util tests
###############################################################################

gspc_util_unit_test (num)

gspc_util_unit_test (remove_prefix)

gspc_util_unit_test (xml_stream)

add_unit_test (NAME gspc_test_util_signal_handler_Execution_can_be_stopped_manually
  SOURCES "signal_handler_Execution_can_be_stopped_manually.cpp"
  LIBRARIES gspc_util
            gspc_logging
)
add_unit_test (NAME gspc_test_util_signal_handler_Execution_can_be_stopped_by_SIGTERM
  SOURCES "signal_handler_Execution_can_be_stopped_by_SIGTERM.cpp"
  LIBRARIES gspc_util
            gspc_logging
)
add_unit_test (NAME gspc_test_util_signal_handler_Execution_can_be_stopped_by_SIGINT
  SOURCES "signal_handler_Execution_can_be_stopped_by_SIGINT.cpp"
  LIBRARIES gspc_util
            gspc_logging
)
add_unit_test (NAME gspc_test_util_signal_handler_Execution_can_be_waited_multiple_times
  SOURCES "signal_handler_Execution_can_be_waited_multiple_times.cpp"
  LIBRARIES gspc_util
            gspc_logging
)
add_unit_test (NAME gspc_test_util_signal_handler_Execution_can_be_stopped_multiple_times
  SOURCES "signal_handler_Execution_can_be_stopped_multiple_times.cpp"
  LIBRARIES gspc_util
            gspc_logging
)
