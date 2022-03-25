#! Assuming \c CMAKE_SOURCE_DIR is a git repository, list all git

# This file is part of GPI-Space.
# Copyright (C) 2022 Fraunhofer ITWM
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

#! submodules checked out there, their path and version in \a FILE.
#! \note Assumes `grep`, `git`, `sort`, `readlink` `sed` to exist and
#! be executable by name alone, i.e. be contained in `$PATH`.
function (list_and_store_git_submodules FILE)
  execute_process (COMMAND git submodule --quiet foreach "echo $(grep -m1 url $(sed \"s,gitdir: ,,\" .git 2>/dev/null || readlink -f .git)/config | sed \"s,.*:,,;s,.git,,\") $sha1"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE _list_git_submodules
    RESULT_VARIABLE _result_list_git_submodules
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (NOT ${_result_list_git_submodules} EQUAL 0)
    message (FATAL_ERROR
      "couldn't list git submodules: ${_result_list_git_submodules}"
    )
  endif()
  execute_process (COMMAND echo "${_list_git_submodules}" COMMAND sort -u
    OUTPUT_VARIABLE _git_submodules
    RESULT_VARIABLE _result_sort_git_submodules
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (NOT ${_result_sort_git_submodules} EQUAL 0)
    message (FATAL_ERROR "couldn't sort git submodules")
  endif()
  write_file ("${FILE}" "${_git_submodules}")
endfunction()

#! Produces the file \a NAME in \c CMAKE_BINARY_DIR either by calling
#! `list_and_store_git_submodules (${CMAKE_BINARY_DIR}/${NAME})`, or
#! if file `${CMAKE_SOURCE_DIR}/${NAME}` exists by copying that
#! without additional checks.
function (list_and_store_git_submodules_if_not_exists NAME)
  if (EXISTS "${CMAKE_SOURCE_DIR}/${NAME}")
    message (STATUS "Using git submodules from file ${CMAKE_SOURCE_DIR}/${NAME}")
    file (COPY "${CMAKE_SOURCE_DIR}/${NAME}" DESTINATION "${CMAKE_BINARY_DIR}")
  else()
    list_and_store_git_submodules ("${CMAKE_BINARY_DIR}/${NAME}")
  endif()
endfunction()

#! Generate the file \a NAME in \c CMAKE_BINARY_DIR as per rules of
#! \see list_and_store_git_submodules_if_not_exists() and install with
#! the same \a NAME to the top level in the project's installation.
function (install_git_submodules_information NAME)
  list_and_store_git_submodules_if_not_exists ("${NAME}")
  install (FILES "${CMAKE_BINARY_DIR}/${NAME}" DESTINATION .)
endfunction()

#! Given two file names \a HERE and \a THERE that have previously been
#! produced with the \see list_and_store_git_submodules() family, ensure
#! that if a submodule exists in both lists, they are using the exact
#! same revision. Submodules that only exist on one side are ignored.
macro (assert_same_git_submodules HERE THERE)
  file (READ "${HERE}" _ours)
  string (REGEX REPLACE "\n" ";" _ours "${_ours}")

  file (READ "${THERE}" _theirs)
  string (REGEX REPLACE " [a-f0-9]*\n" ";" _theirs_names "${_theirs}")
  string (REGEX REPLACE "\n" ";" _theirs "${_theirs}")

  set (_any_mismatch FALSE)

  foreach (_our ${_ours})
    string (REGEX REPLACE " .*" "" _our_name "${_our}")

    if ("${_our_name}" IN_LIST _theirs_names AND NOT "${_our}" IN_LIST _theirs)
      set (_their ${_theirs})
      list (FILTER _their INCLUDE REGEX "${_our_name}.*")
      string (REGEX REPLACE ".* " "" _their_hash "${_their}")
      string (REGEX REPLACE ".* " "" _our_hash "${_our}")

      message (STATUS "submodule ${_our_name} versions differ:")
      message (STATUS " - ours:   ${_our_hash}")
      message (STATUS " - theirs: ${_their_hash}")

      set (_any_mismatch TRUE)
    endif()
  endforeach()

  if (${_any_mismatch})
    message (FATAL_ERROR "At least one git submodule in ${HERE} (ours) and ${THERE} (theirs) differs. See above for the list of mismatches.")
  endif()
endmacro()
