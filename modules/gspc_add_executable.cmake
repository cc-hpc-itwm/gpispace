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

include_guard (GLOBAL)

include (util-cmake/add_macros)
include (util-cmake/parse_arguments)

#! Wrapper for shared/cmake's \see extended_add_executable() which
#! additionally
#! - Replaces the opt-out `NO_RYNAMIC` with the opt-in \a
#!   ENABLE_EXPORTS.
#! - Takes \a RUNTIME to indicate an ~~installed~~ target that's
#!   ~~part of the runtime~~.
#!   \todo \a RUNTIME is misleading: not all installed targets are \a
#!   RUNTIME and not all \a RUNTIME targets are part of the
#!   runtime. Should probably be split into
#!   `PUBLIC`/`PRIVATE`/`INTERNAL` for `INSTALL` and
#!   `INSTALL_DESTINATION` and have \a DONT_APPEND_EXE_SUFFIX separate
#!   of that, or unconditional (and consistent).
function (gspc_add_executable)
  set (options RUNTIME ENABLE_EXPORTS)
  set (one_value_options)
  set (multi_value_options)
  set (required_options)
  _parse_arguments_with_unknown (_arg "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  set (_extra_args)

  if (_arg_RUNTIME)
    list (APPEND _extra_args DONT_APPEND_EXE_SUFFIX INSTALL)
  endif()
  if (NOT _arg_ENABLE_EXPORTS)
    # \todo Should also add `--exclude-libs,ALL` to avoid re-exporting
    # unintended symbols, but e.g. openssl's libssl breaks when doing
    # so. In MR !1115 the decision was made to not do any excluding
    # for now to avoid a misleading potentially broken allowlist.
    list (APPEND _extra_args NO_RDYNAMIC)
  endif()

  extended_add_executable (${_extra_args} ${_arg_UNPARSED_ARGUMENTS})
endfunction()
