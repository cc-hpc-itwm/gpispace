# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

include_guard (GLOBAL)

include (util-cmake/add_macros)
include (util-cmake/parse_arguments)

#! Wrapper for shared/cmake's \see extended_add_library() which
#! additionally
#! - Always adds `VISIBILITY_HIDDEN` for non-`INTERFACE`
#!   libraries.
#! - Only allows installation of `SHARED` or `MODULE` libraries.
function (gspc_add_library)
  set (options INSTALL)
  set (one_value_options TYPE)
  set (multi_value_options)
  set (required_options TYPE)
  _parse_arguments_with_unknown (_arg "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  set (_extra_args)

  set (_allow_installation false)

  # Libraries shall only contain intended symbols, thus hidden
  # visibility, always (except interface libraries which don't contain
  # any objects to hide).
  if (NOT "${_arg_TYPE}" STREQUAL "INTERFACE")
    # \todo Should also add `--exclude-libs,ALL` to avoid re-exporting
    # unintended symbols, but e.g. openssl's libssl breaks when doing
    # so. In MR !1115 the decision was made to not do any excluding
    # for now to avoid a misleading potentially broken allowlist.
    list (APPEND _extra_args VISIBILITY_HIDDEN)
  endif()

  # Libraries shall be self-contained or automatically pull in their
  # dependencies, thus shared.
  if ("${_arg_TYPE}" STREQUAL "SHARED" OR "${_arg_TYPE}" STREQUAL "MODULE")
    set (_allow_installation true)
  endif()

  if (_arg_INSTALL)
    if (NOT _allow_installation)
      message (FATAL_ERROR "Only shared/module libraries shall be installed.")
    endif()
    list (APPEND _extra_args INSTALL)
  endif()

  extended_add_library (${_arg_UNPARSED_ARGUMENTS}
    TYPE ${_arg_TYPE}
    ${_extra_args}
  )
endfunction()
