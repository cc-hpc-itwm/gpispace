#! Determine the Git revision for the repository in \a directory and

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

#! write it to \a output_variable.
#!
#! For the ability to tarball a repository without changing the
#! invocation of this function, the file `${directory}/revision` is
#! checked. If it exists it shall contain the 40 character
#! revision. This file is trusted and no further check will happen.
#!
#! \note Automatically searches for `Git` if not using the
#! override. Not finding `Git` or \a directory not being a git
#! work dir is an error.
function (determine_git_revision directory output_variable_name)
  set (revision_file "${directory}/revision")
  if (EXISTS "${revision_file}")
    file (READ "${revision_file}" output)
    string (LENGTH "${output}" revision_length)
    if (NOT revision_length EQUAL 40)
      message (FATAL_ERROR "Found invalid revision '${output}' in file '${revision_file}'. Delete or update it!")
    endif()
    message (STATUS "Using Git revision '${output}' from file '${revision_file}'. Delete it to force getting it from the git repository.")
  else()
    find_package (Git REQUIRED)

    set (command "${GIT_EXECUTABLE}" rev-parse HEAD)
    execute_process (COMMAND ${command}
      WORKING_DIRECTORY "${directory}"
      OUTPUT_VARIABLE output OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_VARIABLE output ERROR_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE error_code)

    if (NOT ${error_code} EQUAL 0)
      string (REPLACE ";" " " command_string "${command}")
      message (FATAL_ERROR "could not discover revision info for '${directory}': "
        "`${command_string}` failed with error code ${error_code}. "
        "output: ${output}")
    endif()
  endif()

  set (${output_variable_name} "${output}" PARENT_SCOPE)
endfunction()
