#! Safety wrapper around `install (DIRECTORY)`. Copies \a SOURCE into

# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! \a DESTINATION within the installation so that `${SOURCE}/a` ends

# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! up at `${DESTINATION}/a` and not `${DESTINATION}/${SOURCE}/a`.

# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! \note Always uses `USE_SOURCE_PERMISSIONS`.

# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

function (install_directory)
  set (options)
  set (one_value_options DESTINATION SOURCE)
  set (multi_value_options)
  set (required_options DESTINATION SOURCE)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  get_filename_component (real_source "${ARG_SOURCE}" REALPATH)

  install (DIRECTORY "${real_source}/"
    DESTINATION "${ARG_DESTINATION}"
    USE_SOURCE_PERMISSIONS
  )
endfunction()
