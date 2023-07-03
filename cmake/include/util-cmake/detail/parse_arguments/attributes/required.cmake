# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

if (_arg IN_LIST ${_internal}_required)
  message (WARNING
    "${_arg}: multiple definitions of attribute REQUIRED"
  )
else()
  list (APPEND ${_internal}_required ${_arg})
endif()

if (${_internal}_${_arg}_default)
  message (WARNING
    "DEFAULT attribute will be ignored when REQUIRED attribute is set"
  )
endif()
