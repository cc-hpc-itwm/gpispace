#! Assuming \c CMAKE_SOURCE_DIR is a git repository, list all git

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! submodules checked out there, their path and version in \a FILE.

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! \note Assumes `grep`, `git`, `sort`, `readlink` `sed` to exist and

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! be executable by name alone, i.e. be contained in `$PATH`.

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

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

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! `list_and_store_git_submodules (${CMAKE_BINARY_DIR}/${NAME})`, or

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! if file `${CMAKE_SOURCE_DIR}/${NAME}` exists by copying that

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! without additional checks.

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

function (list_and_store_git_submodules_if_not_exists NAME)
  if (EXISTS "${CMAKE_SOURCE_DIR}/${NAME}")
    message (STATUS "Using git submodules from file ${CMAKE_SOURCE_DIR}/${NAME}")
    file (COPY "${CMAKE_SOURCE_DIR}/${NAME}" DESTINATION "${CMAKE_BINARY_DIR}")
  else()
    list_and_store_git_submodules ("${CMAKE_BINARY_DIR}/${NAME}")
  endif()
endfunction()

#! Generate the file \a NAME in \c CMAKE_BINARY_DIR as per rules of

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! \see list_and_store_git_submodules_if_not_exists() and install with

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! the same \a NAME to the top level in the project's installation.

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

function (install_git_submodules_information NAME)
  list_and_store_git_submodules_if_not_exists ("${NAME}")
  install (FILES "${CMAKE_BINARY_DIR}/${NAME}" DESTINATION .)
endfunction()

#! Given two file names \a HERE and \a THERE that have previously been

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! produced with the \see list_and_store_git_submodules() family, ensure

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! that if a submodule exists in both lists, they are using the exact

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! same revision. Submodules that only exist on one side are ignored.

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

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
