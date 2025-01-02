# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

list (POP_FRONT ARGN _arg)
list (APPEND ${_internal}_options ${_arg})

include (${_detail_dir}/parse_attributes.cmake)
