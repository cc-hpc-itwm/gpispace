# This file is part of GPI-Space.
# Copyright (C) 2021 Fraunhofer ITWM
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

###############################################################################
# Checks
###############################################################################
if (NOT BUILD_TESTING OR NOT UTIL_GENERIC_BUILD_TESTING)
  return ()
endif ()

###############################################################################
# Includes
###############################################################################

include (util-cmake/add_unit_test)

###############################################################################
# Definitions
###############################################################################

function (util_generic_unit_test _path)
  _parse_arguments_with_unknown (ARG
    "" "" "LIBRARIES;SOURCES;REQUIRE_FAIL" "" ${ARGN}
  )
  string (REPLACE "/" "-" _name "util-generic-${_path}")

  if (NOT ARG_REQUIRE_FAIL)
    add_unit_test (NAME "${_name}"
      SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${_path}.cpp" ${ARG_SOURCES}
      USE_BOOST
      LIBRARIES Util::Generic ${ARG_LIBRARIES}
      ${ARG_UNPARSED_ARGUMENTS}
    )
  else()
    # \todo move REQUIRE_FAIL support one level down to not recompile
    # for every case.
    foreach (_case ${ARG_REQUIRE_FAIL})
      add_unit_test (NAME "${_name}-${_case}"
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${_path}.cpp" ${ARG_SOURCES}
        USE_BOOST
        LIBRARIES Util::Generic ${ARG_LIBRARIES}
        ARGS --run_test="${_case}"
        ${ARG_UNPARSED_ARGUMENTS}
      )
      set_tests_properties ("${_name}-${_case}" PROPERTIES WILL_FAIL true)
    endforeach()
  endif()
endfunction()

###############################################################################
# Test declarations
###############################################################################

util_generic_unit_test (asynchronous)

util_generic_unit_test (blocked)

util_generic_unit_test (callable_signature)

util_generic_unit_test (chrono_unit_for_period)

util_generic_unit_test (connectable_to_address_string)

util_generic_unit_test (cxx17/apply)

util_generic_unit_test (cxx17/holds_alternative)

util_generic_unit_test (cxx17/logical_operator_type_traits)

util_generic_unit_test (cxx17/void_t)

util_generic_unit_test (divru)

if (BUILD_TESTING)
  extended_add_library (NAME util-generic-dynamic_linking-test_library
    TYPE SHARED
    SOURCES "dynamic_linking-lib.cpp"
    LIBRARIES Util::Generic
    INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
  )
  set_property (TARGET util-generic-dynamic_linking-test_library
    PROPERTY CXX_VISIBILITY_PRESET hidden
  )
endif()

util_generic_unit_test (dynamic_linking
  DEPENDS util-generic-dynamic_linking-test_library
  COMPILE_DEFINITIONS PRIVATE
    "-DFHG_UTIL_TESTING_LIB_PATH=\"$<TARGET_FILE:util-generic-dynamic_linking-test_library>\""
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (executable_path)

util_generic_unit_test (fallthrough)

util_generic_unit_test (filesystem_lock_directory)

util_generic_unit_test (finally)

util_generic_unit_test (first_then)

util_generic_unit_test (functor_visitor)

util_generic_unit_test (getenv)

util_generic_unit_test (hard_integral_typedef)

util_generic_unit_test (hash/combined_hash)

util_generic_unit_test (hash/combined_hash-performance PERFORMANCE_TEST)

util_generic_unit_test (hostname)

util_generic_unit_test (in_this_scope)

util_generic_unit_test (join)

util_generic_unit_test (latch RUN_SERIAL)

util_generic_unit_test (make_optional)

util_generic_unit_test (mp)

util_generic_unit_test (ndebug)

util_generic_unit_test (nest_exceptions)

util_generic_unit_test (ostream/callback/bracket)

util_generic_unit_test (ostream/callback/close)

util_generic_unit_test (ostream/callback/function)

util_generic_unit_test (ostream/callback/open)

util_generic_unit_test (ostream/callback/print)

util_generic_unit_test (ostream/callback/range)

util_generic_unit_test (ostream/echo)

util_generic_unit_test (ostream/line_by_line)

util_generic_unit_test (ostream/modifier)

util_generic_unit_test (ostream/prefix_per_line)

util_generic_unit_test (ostream/put_time)

util_generic_unit_test (ostream/redirect)

util_generic_unit_test (ostream/redirect_standard_streams)

util_generic_unit_test (ostream/to_string)

util_generic_unit_test (print_container)

util_generic_unit_test (print_exception)

util_generic_unit_test (procfs
  ARGS --some args --to actually --test something, yeah
)

util_generic_unit_test (boost/program_options/generic
  LIBRARIES Boost::program_options
)

util_generic_unit_test (boost/program_options/separated_argument_list_parser
  LIBRARIES Boost::program_options
)

util_generic_unit_test (read_file)

util_generic_unit_test (read_lines)

util_generic_unit_test (refcounted_set)

util_generic_unit_test (scoped_boost_asio_io_service_with_threads
  LIBRARIES Boost::thread
)

util_generic_unit_test (scoped_file_with_content)

util_generic_unit_test (serialization/boost/asio/ip/tcp/endpoint)

util_generic_unit_test (serialization/boost/asio/local/stream_protocol)

util_generic_unit_test (serialization/boost/blank)

util_generic_unit_test (serialization/boost/filesystem/path)

util_generic_unit_test (serialization/by_member)

util_generic_unit_test (serialization/exception)

util_generic_unit_test (serialization/std/chrono)

util_generic_unit_test (serialization/std/error_code)

util_generic_unit_test (serialization/std/tuple)

util_generic_unit_test (serialization/trivial)

util_generic_unit_test (split)

if (BUILD_TESTING)
  extended_add_executable (NAME system_command
    SOURCES "system_command.cpp"
    LIBRARIES Util::Generic
  )
endif()

util_generic_unit_test (system
  LIBRARIES Boost::filesystem
  DEPENDS system_command.exe
  ARGS "$<TARGET_FILE:system_command.exe>"
)

util_generic_unit_test (temporary_file)

util_generic_unit_test (temporary_path)

util_generic_unit_test (temporary_return_value_holder)

util_generic_unit_test (testing/measure_average_time PERFORMANCE_TEST)

util_generic_unit_test (testing/printer/chrono
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/future
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/generic
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/hard_integral_typedef
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/list
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/map
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/multimap
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/multiset
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/optional
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/pair
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/set
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/tuple
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/type_info
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/printer/vector
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/test"
)

util_generic_unit_test (testing/random)

util_generic_unit_test (testing/random_string)

util_generic_unit_test (testing/require_compiletime)

util_generic_unit_test (testing/require_container_is_permutation)
util_generic_unit_test (testing/require_container_is_permutation_negative
  REQUIRE_FAIL different_size_is_noticed
               different_content_is_noticed
)

util_generic_unit_test (testing/require_exception)

util_generic_unit_test (testing/require_type)

util_generic_unit_test (this_bound_mem_fn)

util_generic_unit_test (threadsafe_queue RUN_SERIAL)

util_generic_unit_test (timer)

util_generic_unit_test (va_args)

util_generic_unit_test (variant_cast)

util_generic_unit_test (wait_and_collect_exceptions)

util_generic_unit_test (write_file)
