###############################################################################
# Includes
###############################################################################

include (CheckCXXCompilerFlag)
include (CheckCXXSourceRuns)
include (CheckFunctionExists)

###############################################################################
# Build checks
###############################################################################

check_function_exists (signalfd FUNCTION_EXISTS_SIGNALFD)
check_function_exists (pipe2 FUNCTION_EXISTS_PIPE2)

check_cxx_compiler_flag ("-Werror=old-style-cast"
  COMPILER_FLAG_EXISTS_WERROR_OLD_STYLE_CAST
)

if (COMPILER_FLAG_EXISTS_WERROR_OLD_STYLE_CAST)
  set (CMAKE_REQUIRED_FLAGS_SAVED "${CMAKE_REQUIRED_FLAGS}")
  set (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Werror=old-style-cast")
  check_cxx_source_runs ([[
    #include <sys/wait.h>
    int main (int argc, char**)
    {
      return WEXITSTATUS (argc);
      //     ^^^^^^^^^^^
    }
    ]]
    HAS_WEXITSTATUS_WITHOUT_OLD_STYLE_CAST
  )
  check_cxx_source_runs ([[
    #include <signal.h>
    int main (int, char**)
    {
      return SIG_ERR == nullptr;
      //     ^^^^^^^
    }
    ]]
    HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST
  )
  check_cxx_source_runs ([[
    #include <sys/mman.h>
    int main (int, char**)
    {
      return MAP_FAILED == nullptr;
      //     ^^^^^^^^^^
    }
    ]]
    HAS_MAP_FAILED_WITHOUT_OLD_STYLE_CAST
  )
  set (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_SAVED}")
else()
  set (HAS_WEXITSTATUS_WITHOUT_OLD_STYLE_CAST 1)
  set (HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST 1)
  set (HAS_MAP_FAILED_WITHOUT_OLD_STYLE_CAST 1)
endif()

###############################################################################
# Library
###############################################################################

extended_add_library (NAME gspc_util
  TYPE SHARED
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE ${CMAKE_SOURCE_DIR}/include
  SOURCES
    "chrono_unit_for_period.cpp"
    "connectable_to_address_string.cpp"
    "dynamic_linking.cpp"
    "executable_path.cpp"
    "exit_status.cpp"
    "filesystem_lock_directory.cpp"
    "hostname.cpp"
    "$<$<BOOL:${FUNCTION_EXISTS_SIGNALFD}>:interruption_handler.cpp>"
    "map_failed.cpp"
    "ostream/echo.cpp"
    "ostream/line_by_line.cpp"
    "ostream/modifier.cpp"
    "ostream/prefix_per_line.cpp"
    "ostream/redirect.cpp"
    "ostream/redirect_standard_streams.cpp"
    "print_exception.cpp"
    "procfs.cpp"
    "boost/program_options/separated_argument_list_parser.cpp"
    "read_file.cpp"
    "read_lines.cpp"
    "sig_err.cpp"
    "serialization/exception.cpp"
    "serialization/std/variant.cpp"
    "syscall.cpp"
    "system.cpp"
    "syscall/directory.cpp"
    "syscall/process_signal_block.cpp"
    "syscall/signal_fd.cpp"
    "syscall/signal_set.cpp"
    "temporary_file.cpp"
    "temporary_path.cpp"
    "wait_and_collect_exceptions.cpp"
    "write_file.cpp"
    "backtracing_exception.cpp"
    "cctype.cpp"
    "indenter.cpp"
    "num.cpp"
    "read_bool.cpp"
    "remove_prefix.cpp"
    "signal_handler_manager.cpp"
    "starts_with.cpp"
    "system_with_blocked_SIGCHLD.cpp"
    "xml.cpp"
    "parse/error.cpp"
    "parse/position.cpp"
    "parse/require.cpp"
    "num/show.cpp"
    "cpp/block.cpp"
    "cpp/include.cpp"
    "cpp/include_guard.cpp"
    "cpp/namespace.cpp"
    "cpp/struct.cpp"
  LIBRARIES
    PUBLIC Boost::base
    PUBLIC Boost::serialization
    PUBLIC Boost::unit_test_framework
    PUBLIC Boost::system
    PUBLIC Boost::thread
    PUBLIC ${CMAKE_DL_LIBS}
    #! \todo find librt and only link if required by shm_open+
    rt
    Threads::Threads
    PUBLIC fmt
    gspc_assert
    GPISpace::header-only
  COMPILE_DEFINITIONS
    FUNCTION_EXISTS_SIGNALFD=$<BOOL:${FUNCTION_EXISTS_SIGNALFD}>
    FUNCTION_EXISTS_PIPE2=$<BOOL:${FUNCTION_EXISTS_PIPE2}>
    HAS_WEXITSTATUS_WITHOUT_OLD_STYLE_CAST=$<BOOL:${HAS_WEXITSTATUS_WITHOUT_OLD_STYLE_CAST}>
    HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST=$<BOOL:${HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST}>
    HAS_MAP_FAILED_WITHOUT_OLD_STYLE_CAST=$<BOOL:${HAS_MAP_FAILED_WITHOUT_OLD_STYLE_CAST}>
  POSITION_INDEPENDENT
  INSTALL
    CREATE_BUNDLE_INFO
)

###############################################################################
# Install headers
###############################################################################

install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/asynchronous.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/asynchronous.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/backtracing_exception.fwd.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/backtracing_exception.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/blocked.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/blocked.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/callable_signature.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/callable_signature.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cctype.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/chrono_unit_for_period.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/chrono_unit_for_period.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/connectable_to_address_string.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/connectable_to_address_string.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/divru.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/dynamic_linking.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/dynamic_linking.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/exception_printer.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/executable_path.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/exit_status.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fallthrough.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/filesystem_lock_directory.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/finally.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/first_then.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/functor_visitor.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/hard_integral_typedef.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/hard_integral_typedef.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/hostname.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/in_this_scope.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/in_this_scope.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/indenter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/interruption_handler.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/join.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/join.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/join.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/latch.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/latch.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/make_optional.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/make_optional.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/map_failed.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ndebug.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/next.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/num.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/print_container.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/print_exception.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/procfs.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/read_bool.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/read_file.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/read_lines.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/refcounted_set.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/remove_prefix.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/scoped_boost_asio_io_service_with_threads.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/scoped_file_with_content.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/sig_err.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/signal_handler_manager.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/split.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/starts_with.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/syscall.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/system.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/system.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/system_with_blocked_SIGCHLD.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/temporary_file.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/temporary_path.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/this_bound_mem_fn.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/threadsafe_queue.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/unreachable.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/unused.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/va_args.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/wait_and_collect_exceptions.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/warning.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/write_file.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/xml.fwd.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/xml.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/generic.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/generic.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/require_all_if_one.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/separated_argument_list_parser.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/boost/program_options"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/executable.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/existing_directory.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/existing_directory.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/existing_path.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/existing_path.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/integral_greater_than.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/integral_greater_than.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/is_directory_if_exists.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/is_directory_if_exists.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/nonempty_file.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/nonempty_file.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/nonempty_string.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/nonexisting_path.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/nonexisting_path_in_existing_directory.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/nonexisting_path_in_existing_directory.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/positive_integral.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/boost/program_options/validators/ratio.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/boost/program_options/validators"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cpp/block.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cpp/include.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cpp/include_guard.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cpp/namespace.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cpp/struct.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/cpp"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cxx17/future_error.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cxx17/future_error.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cxx17/holds_alternative.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cxx17/holds_alternative.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/cxx17/void_t.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/cxx17"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/boost/blank.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/boost/variant.formatter.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/fmt/boost"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/boost/detail/optional.formatter.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/boost/detail/variant.formatter.ipp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/fmt/boost/detail"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/boost/multiprecision/cpp_int.formatter.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/fmt/boost/multiprecision"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/std/exception.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/std/optional.formatter.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/std/variant.formatter.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/fmt/std"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/std/detail/exception.formatter.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/std/detail/optional.formatter.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/std/detail/variant.formatter.ipp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/fmt/std/detail"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/std/filesystem/path.formatter.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/fmt/std/filesystem"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/fmt/std/filesystem/detail/path.formatter.ipp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/fmt/std/filesystem/detail"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/hash/combined_hash.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/hash"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/hash/asio/ip/tcp/endpoint.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/hash/asio/ip/tcp"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/hash/boost/variant.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/hash/boost"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/hash/std/pair.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/hash/std/tuple.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/hash/std"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/mp/exactly_one_is.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/mp/none_is.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/mp"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/num/show.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/num"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/echo.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/line_by_line.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/modifier.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/prefix_per_line.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/put_time.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/redirect.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/redirect_standard_streams.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/to_string.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/ostream"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/callback/bracket.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/callback/close.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/callback/function.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/callback/open.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/callback/print.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/ostream/callback/range.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/ostream/callback"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/parse/error.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/parse/position.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/parse/require.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/parse"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/access.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/access.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/by_member.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/by_member.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/exception.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/exception.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/trivial.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/trivial.ipp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/serialization"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/asio/ip/tcp/endpoint.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/serialization/asio/ip/tcp"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/asio/local/stream_protocol.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/serialization/asio/local"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/boost/blank.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/serialization/boost"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/detail/base_class.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/serialization/detail"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/std/chrono.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/std/error_code.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/std/optional.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/std/tuple.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/std/variant.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/serialization/std"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/serialization/std/filesystem/path.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/serialization/std/filesystem"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/syscall/directory.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/syscall/process_signal_block.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/syscall/signal_fd.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/syscall/signal_set.hpp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/syscall"
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/util/timer/application.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/timer/application.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/timer/scoped.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/timer/scoped.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/timer/sections.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/util/timer/sections.ipp"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/util/timer"
)

###############################################################################
# Qt utilities
###############################################################################

add_subdirectory (qt)
