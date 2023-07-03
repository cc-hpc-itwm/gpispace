# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

set (_arg "HELP")
set (${_internal}_${_type} ON)
list (APPEND ${_internal}_options ${_arg})

if (${_internal}_${_arg}_description)
  message (WARNING
    "${_arg}: multiple definitions of attribute DESCRIPTION"
  )
else()
  set (${_internal}_${_arg}_description "Displays this message")
endif()
