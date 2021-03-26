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

cmake_policy (SET CMP0057 NEW) # "Support new IN_LIST if() operator."

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

function (list_and_store_git_submodules_if_not_exists NAME)
  if (EXISTS "${CMAKE_SOURCE_DIR}/${NAME}")
    message (STATUS "Using git submodules from file ${CMAKE_SOURCE_DIR}/${NAME}")
    file (COPY "${CMAKE_SOURCE_DIR}/${NAME}" DESTINATION "${CMAKE_BINARY_DIR}")
  else()
    list_and_store_git_submodules ("${CMAKE_BINARY_DIR}/${NAME}")
  endif()
endfunction()

function (install_git_submodules_information NAME)
  list_and_store_git_submodules_if_not_exists ("${NAME}")
  install (FILES "${CMAKE_BINARY_DIR}/${NAME}" DESTINATION .)
endfunction()

macro (assert_same_git_submodules HERE THERE)
  cmake_policy (SET CMP0057 NEW)

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
