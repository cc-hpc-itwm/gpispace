# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

if (ARGN)
  list (GET ARGN 0 _next)
  string (TOLOWER "${_next}" _next)
endif()

while (_next IN_LIST ${_internal}_attributes)
  list (POP_FRONT ARGN)
  include (${_attributes_dir}/${_next}.cmake)

  if (NOT ARGN)
    break()
  endif()

  list (GET ARGN 0 _next)
  string (TOLOWER "${_next}" _next)
endwhile()
