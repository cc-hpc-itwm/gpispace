#! Safety wrapper around `install (DIRECTORY)`. Copies \a SOURCE into

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

#! \a DESTINATION within the installation so that `${SOURCE}/a` ends
#! up at `${DESTINATION}/a` and not `${DESTINATION}/${SOURCE}/a`.
#! \note Always uses `USE_SOURCE_PERMISSIONS`.
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
