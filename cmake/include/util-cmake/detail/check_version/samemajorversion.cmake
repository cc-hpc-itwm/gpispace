# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

if (_base_version VERSION_LESS_EQUAL _given_version)
  util_cmake_split_version (_base_version ${_base_version})
  list (GET _base_version 0 _base_major)

  util_cmake_split_version (_given_version ${_given_version})
  list (GET _given_version 0 _given_major)

  if (_base_major EQUAL _given_major)
    set (_result true)
  endif()
endif()
