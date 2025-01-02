# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

if (_type STREQUAL "option")
  message (WARNING
    "argument type OPTION ignores DEFAULT attribute"
  )
endif()

if (${_internal}_${_arg}_default)
  message (WARNING
    "${_arg}: multiple definitions of attribute DEFAULT"
  )
endif()

if (_arg IN_LIST ${_internal}_required)
  message (WARNING
    "DEFAULT attribute will be ignored when REQUIRED attribute is set"
  )
endif()

list (POP_FRONT ARGN _default)
set (${_internal}_${_arg}_default ${_default})

if (_type STREQUAL "multi_value" AND ARGN)
  list (GET ARGN 0 _next)
  string (TOLOWER "${_next}" _next)

  while (NOT _next IN_LIST ${_internal}_attributes AND NOT _next IN_LIST ${_internal}_types)
    list (POP_FRONT ARGN _default)
    list (APPEND ${_internal}_${_arg}_default ${_default})

    if (NOT ARGN)
      break()
    endif()

    list (GET ARGN 0 _next)
    string (TOLOWER "${_next}" _next)
  endwhile()
endif()
