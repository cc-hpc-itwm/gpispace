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

extended_add_library (NAME Generic
  NAMESPACE Util
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
    "read_lines.cpp"
    "sig_err.cpp"
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
  LIBRARIES
    Util::Generic-Headers
    Boost::base
    Boost::serialization
    Boost::filesystem
    Boost::unit_test_framework
    Boost::system
    ${CMAKE_DL_LIBS}
    #! \todo find librt and only link if required by shm_open+
    rt
    Threads::Threads
  COMPILE_DEFINITIONS
    FUNCTION_EXISTS_SIGNALFD=$<BOOL:${FUNCTION_EXISTS_SIGNALFD}>
    FUNCTION_EXISTS_PIPE2=$<BOOL:${FUNCTION_EXISTS_PIPE2}>
    HAS_WEXITSTATUS_WITHOUT_OLD_STYLE_CAST=$<BOOL:${HAS_WEXITSTATUS_WITHOUT_OLD_STYLE_CAST}>
    HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST=$<BOOL:${HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST}>
    HAS_MAP_FAILED_WITHOUT_OLD_STYLE_CAST=$<BOOL:${HAS_MAP_FAILED_WITHOUT_OLD_STYLE_CAST}>
  POSITION_INDEPENDENT
  INSTALL
)
target_compile_features (Util-Generic
  PUBLIC "${_project_cxx_std}"
)
