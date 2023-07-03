#! Determine the Git revision for the repository in \a directory and

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! write it to \a output_variable.

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#!

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! For the ability to tarball a repository without changing the

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! invocation of this function, the file `${directory}/revision` is

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! checked. If it exists it shall contain the 40 character

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! revision. This file is trusted and no further check will happen.

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#!

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! \note Automatically searches for `Git` if not using the

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! override. Not finding `Git` or \a directory not being a git

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! work dir is an error.

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

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
