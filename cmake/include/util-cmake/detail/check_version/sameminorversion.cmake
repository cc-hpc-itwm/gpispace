# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

if (_base_version VERSION_LESS_EQUAL _given_version)
  util_cmake_split_version (_base_version ${_base_version})
  list (GET _base_version 0 _base_major)
  list (GET _base_version 1 _base_minor)

  util_cmake_split_version (_given_version ${_given_version})
  list (GET _given_version 0 _given_major)
  list (GET _given_version 1 _given_minor)

  if (_base_major EQUAL _given_major AND _base_minor EQUAL _given_minor)
    set (_result true)
  endif()
endif()