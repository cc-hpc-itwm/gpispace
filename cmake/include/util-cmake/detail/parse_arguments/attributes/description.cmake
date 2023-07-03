# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

list (POP_FRONT ARGN _description)

if (${_internal}_${_arg}_description)
  message (WARNING
    "${_arg}: multiple definitions of attribute DESCRIPTION"
  )
else()
  set (${_internal}_${_arg}_description "${_description}")
endif()
