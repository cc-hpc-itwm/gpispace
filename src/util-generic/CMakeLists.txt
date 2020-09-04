# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
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

cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

find_package (Threads REQUIRED)

include (add_macros)

include (beautify_find_boost)
find_boost (1.59 REQUIRED QUIET
  COMPONENTS filesystem
             program_options
             serialization
             thread
)

include (CheckCXXCompilerFlag)
include (CheckCXXSourceRuns)
include (CheckFunctionExists)

check_function_exists (signalfd FUNCTION_EXISTS_SIGNALFD)
check_function_exists (pipe2 FUNCTION_EXISTS_PIPE2)

# libstcxx unconditionally adds the std::hash<Enum> overload [1],
# which should be cxx14 only, in newer versions. Since we also have
# some hosts with old libstdcxx where it has not yet been added, we
# still provide our own implementation. The maintainer has confirmed
# that this is intended to provide convenience, even in c++11 mode, so
# we just #if out the entire file if we detect the overload.
# [1] https://gcc.gnu.org/ml/gcc-patches/2015-07/msg02162.html
check_cxx_source_runs ([[
  #include <functional>
  enum E : int {};
  int main (int, char**)
  {
    return (std::hash<E>{} (E{}) == std::hash<int>{} (int{})) ? 0 : 1;
    //      ^^^^^^^^^^^^
  }
  ]]
  HAS_STD_HASH_FOR_ENUM
)

check_cxx_source_runs ([[
  #include <memory>
  int main (int, char**)
  {
    return *std::make_unique<int> (3) == 3 ? 0 : 1;
    //      ^^^^^^^^^^^^^^^^
  }
  ]]
  HAS_STD_MAKE_UNIQUE
)

check_cxx_source_runs ([[
  #include <utility>
  using via_make = std::make_index_sequence<3>;
  //               ^^^^^^^^^^^^^^^^^^^^^^^^
  using via_expl = std::integer_sequence<std::size_t, 0, 1, 2>;
  //               ^^^^^^^^^^^^^^^^^^^^^
  int main (int, char**)
  {
    return via_make::size() == via_expl::size() ? 0 : 1;
  }
  ]]
  HAS_STD_INTEGER_SEQUENCE
)

check_cxx_source_runs ([[
  #include <tuple>
  int main (int, char**)
  {
    std::tuple<float, int> const t (2.62f, 262);
    return std::get<int> (t) == 262 ? 0 : 1;
    //     ^^^^^^^^^^^^^
  }
  ]]
  HAS_STD_GET_BY_TYPE
)

check_cxx_source_runs ([[
  #include <type_traits>
  int main (int, char**)
  {
    return std::negation<std::conjunction<std::false_type>>{} ? 0 : 1;
    //     ^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^
  }
  ]]
  HAS_STD_LOGICAL_OPERATOR_TYPE_TRAITS
)

check_cxx_source_runs ([[
  #include <type_traits>
  int main (int, char**)
  {
  //! \note Clang interprets the standard differently than everyone
  //! else (https://wg21.cmeerw.net/cwg/issue1558), making the "select
  //! the matching void_t" pattern impossible: there can always be at
  //! most one void_t usage, regardless of SFINAE. Our
  //! emulation-implementation does that exact workaround, so isn't
  //! true c++17, making it work regardless of compiler.
  #if defined(__clang__)
    return 1;
  #else
    return std::is_same<std::void_t<int>, void>{} ? 0 : 1;
    //                  ^^^^^^^^^^^
  #endif
  }
  ]]
  HAS_STD_VOID_T
)

check_cxx_source_runs ([[
  #include <functional>
  #include <tuple>
  int main (int, char**)
  {
    return std::apply (std::minus<int>{}, std::make_tuple (3, 3));
    //     ^^^^^^^^^^
  }
  ]]
  HAS_STD_APPLY
)

check_cxx_compiler_flag ("-Wundefined-func-template"
  COMPILER_FLAG_EXISTS_WUNDEFINED_FUNC_TEMPLATE
)

extended_add_library (NAME Generic
  NAMESPACE Util
  SOURCES "chrono_unit_for_period.cpp"
          "connectable_to_address_string.cpp"
          "dynamic_linking.cpp"
          "executable_path.cpp"
          "filesystem_lock_directory.cpp"
          "hostname.cpp"
          "$<$<BOOL:${FUNCTION_EXISTS_SIGNALFD}>:interruption_handler.cpp>"
          "ostream/modifier.cpp"
          "ostream/prefix_per_line.cpp"
          "print_exception.cpp"
          "procfs.cpp"
          "read_lines.cpp"
          "serialization/exception.cpp"
          "syscall.cpp"
          "system.cpp"
          "syscall/directory.cpp"
          "syscall/process_signal_block.cpp"
          "syscall/signal_fd.cpp"
          "syscall/signal_set.cpp"
          "testing/random.cpp"
          "testing/random/char.cpp"
          "testing/random/string.cpp"
          "wait_and_collect_exceptions.cpp"
  LIBRARIES Boost::base
            Boost::serialization
            Boost::filesystem
            ${CMAKE_DL_LIBS}
            #! \todo find librt and only link if required by shm_open+
            rt
            Threads::Threads
  COMPILE_DEFINITIONS -DFUNCTION_EXISTS_SIGNALFD=$<BOOL:${FUNCTION_EXISTS_SIGNALFD}>
                      -DFUNCTION_EXISTS_PIPE2=$<BOOL:${FUNCTION_EXISTS_PIPE2}>
                      -DHAS_STD_APPLY=$<BOOL:${HAS_STD_APPLY}>
                      -DHAS_STD_GET_BY_TYPE=$<BOOL:${HAS_STD_GET_BY_TYPE}>
                      -DHAS_STD_HASH_FOR_ENUM=$<BOOL:${HAS_STD_HASH_FOR_ENUM}>
                      -DHAS_STD_INTEGER_SEQUENCE=$<BOOL:${HAS_STD_INTEGER_SEQUENCE}>
                      -DHAS_STD_LOGICAL_OPERATOR_TYPE_TRAITS=$<BOOL:${HAS_STD_LOGICAL_OPERATOR_TYPE_TRAITS}>
                      -DHAS_STD_MAKE_UNIQUE=$<BOOL:${HAS_STD_MAKE_UNIQUE}>
                      -DHAS_STD_VOID_T=$<BOOL:${HAS_STD_VOID_T}>
                      -DCOMPILER_FLAG_EXISTS_WUNDEFINED_FUNC_TEMPLATE=$<BOOL:${COMPILER_FLAG_EXISTS_WUNDEFINED_FUNC_TEMPLATE}>
  POSITION_INDEPENDENT
)


function (util_generic_unit_test _path)
  _parse_arguments_with_unknown (ARG
    "" "" "LIBRARIES;SOURCES;REQUIRE_FAIL" "" ${ARGN}
  )
  string (REPLACE "/" "-" _name "util-generic-${_path}")

  if (NOT ARG_REQUIRE_FAIL)
    add_unit_test (NAME "${_name}"
      SOURCES "test/${_path}.cpp" ${ARG_SOURCES}
      USE_BOOST
      LIBRARIES Util::Generic ${ARG_LIBRARIES}
      ${ARG_UNPARSED_ARGUMENTS}
    )
  else()
    # \todo move REQUIRE_FAIL support one level down to not recompile
    # for every case.
    foreach (_case ${ARG_REQUIRE_FAIL})
      add_unit_test (NAME "${_name}-${_case}"
        SOURCES "test/${_path}.cpp" ${ARG_SOURCES}
        USE_BOOST
        LIBRARIES Util::Generic ${ARG_LIBRARIES}
        ARGS --run_test="${_case}"
        ${ARG_UNPARSED_ARGUMENTS}
      )
      set_tests_properties ("${_name}-${_case}" PROPERTIES WILL_FAIL true)
    endforeach()
  endif()
endfunction()

util_generic_unit_test (asynchronous)

util_generic_unit_test (blocked)

util_generic_unit_test (callable_signature)

util_generic_unit_test (chrono_unit_for_period)

util_generic_unit_test (connectable_to_address_string)

util_generic_unit_test (cxx14/enum_hash)

util_generic_unit_test (cxx14/get_by_type)

util_generic_unit_test (cxx14/integer_sequence)

util_generic_unit_test (cxx14/make_unique)

util_generic_unit_test (cxx17/apply)

util_generic_unit_test (cxx17/logical_operator_type_traits)

util_generic_unit_test (cxx17/void_t)

util_generic_unit_test (divru)

extended_add_library (NAME util-generic-dynamic_linking-test_library
  TYPE SHARED
  SOURCES "test/dynamic_linking-lib.cpp"
  LIBRARIES Util::Generic
)
set_property (TARGET util-generic-dynamic_linking-test_library
  PROPERTY CXX_VISIBILITY_PRESET hidden
)
util_generic_unit_test (dynamic_linking
  DEPENDS util-generic-dynamic_linking-test_library
  COMPILE_DEFINITIONS PRIVATE
    "-DFHG_UTIL_TESTING_LIB_PATH=\"$<TARGET_FILE:util-generic-dynamic_linking-test_library>\""
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

util_generic_unit_test (program_options/separated_argument_list_parser
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

extended_add_executable (NAME system_command
  SOURCES "test/system_command.cpp"
  LIBRARIES Util::Generic
)

util_generic_unit_test (system
  LIBRARIES Boost::filesystem
  DEPENDS system_command.exe
  ARGS "$<TARGET_FILE:system_command.exe>"
)

util_generic_unit_test (temporary_file)

util_generic_unit_test (temporary_path)

util_generic_unit_test (temporary_return_value_holder)

util_generic_unit_test (testing/measure_average_time PERFORMANCE_TEST)

util_generic_unit_test (testing/printer/chrono)

util_generic_unit_test (testing/printer/future)

util_generic_unit_test (testing/printer/generic)

util_generic_unit_test (testing/printer/hard_integral_typedef)

util_generic_unit_test (testing/printer/list)

util_generic_unit_test (testing/printer/map)

util_generic_unit_test (testing/printer/multimap)

util_generic_unit_test (testing/printer/multiset)

util_generic_unit_test (testing/printer/optional)

util_generic_unit_test (testing/printer/pair)

util_generic_unit_test (testing/printer/set)

util_generic_unit_test (testing/printer/tuple)

util_generic_unit_test (testing/printer/type_info)

util_generic_unit_test (testing/printer/vector)

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
