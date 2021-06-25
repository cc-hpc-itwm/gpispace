# This file is part of GPI-Space.
# Copyright (C) 2021 Fraunhofer ITWM
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

include (CMakeParseArguments)

#! Equivalent to CMakeParseArguments's `cmake_parse_arguments()`,
#! except that it forbids unknown arguments and allows to specify
#! required arguments.
#!
#! The function will parse all tokens given after the named arguments
#! and populate variables named like the defined options, prefixed
#! with the given \a _prefix, i.e. if `_prefix=_arg` and an option
#! `BOGGLE` is defined, `_arg_BOGGLE` will be set. It is convention
#! that argument names are upper case. Note that it is impossible to
#! pass argument values that are also an argument name.
#!
#! If an argument given is unknown, the function will error out. Use
#! \see _parse_arguments_with_unknown() to get a list of unknown
#! tokens. Also see the note about parsing quirks there.
#!
#! There are three categories of arguments. There is no value type
#! specified or checked. The arguments are given to the function in
#! three lists:
#! - \a _options: `[<option_name>]` - Boolean options. True if given,
#!   false if not. May appear multiple times.
#! - \a _one_value_options: `[<option_name> <value>]` - Classic named
#!   arguments. If given, has to be followed by a single value. Should
#!   appear only once, but last occurence will define value.
#! - \a _multi_value_options: `[<option_name> [<value>]...]` -
#!   Arguments with zero to infinite values resulting in a list. May
#!   appear multiple times, the lists are joined. A value list ends
#!   either at the end of the argument list or at the next known
#!   option name.
#!
#! After the definition of arguments, a fourth list defines all
#! argument names that are required to exist. While it usually makes
#! no sense to enforce existence of boolean options (just hardcode
#! TRUE then), they can still be added for sake of legacy
#! compatibility of an option becoming mandatory but not wanting to
#! break API.
#!
#! A usual invokation looks like
#! ```
#!   function (ex1)
#!     set (options RUNTIME ENABLE_EXPORTS)
#!     set (one_value_options NAME)
#!     set (multi_value_options LIBRARIES)
#!     set (required_options NAME)
#!     _parse_arguments (_arg "${options}" "${one_value_options}"
#!       "${multi_value_options}" "${required_options}" ${ARGN})
#!     message (STATUS "NAME=${_arg_NAME} RUNTIME=${_arg_RUNTIME}
#!       ENABLE_EXPORTS=${_arg_ENABLE_EXPORTS} LIBRARIES=${_arg_LIBRARIES}")
#! ```
#! but having the list inlined is of course also possible. The lists
#! of options may also be empty:
#! ```
#!   function (ex2)
#!     _parse_arguments (_arg "RUNTIME;ENABLE_EXPORTS" "" "LIBRARIES" "" ${ARGN})
#!     message (STATUS "RUNTIME=${_arg_RUNTIME}
#!       ENABLE_EXPORTS=${_arg_ENABLE_EXPORTS} LIBRARIES=${_arg_LIBRARIES}")
#! ```
#! These functions can then be called as follows:
#! ```
#!   # error: required argument NAME missing
#!   ex1()
#!   # NAME=a, RUNTIME=FALSE ENABLE_EXPORTS=FALSE LIBRARIES=
#!   ex1 (NAME a)
#!   # NAME=a RUNTIME=TRUE ENABLE_EXPORTS=FALSE LIBRARIES=
#!   ex1 (NAME a RUNTIME)
#!   # error: required argument NAME missing
#!   ex1 (NAME RUNTIME)
#!   # NAME=b, RUNTIME=TRUE ENABLE_EXPORTS=FALSE LIBRARIES=
#!   ex1 (NAME RUNTIME NAME b)
#!   # NAME=a RUNTIME=TRUE ENABLE_EXPORTS=FALSE LIBRARIES=boggle;noggle
#!   ex1 (NAME a LIBRARIES boggle noggle RUNTIME)
#!   # RUNTIME=FALSE ENABLE_EXPORTS=FALSE LIBRARIES=
#!   ex2()
#!   # RUNTIME=TRUE ENABLE_EXPORTS=TRUE LIBRARIES=a;a;b
#!   ex2 (RUNTIME ENABLE_EXPORTS LIBRARIES a ENABLE_EXPORTS LIBRARIES a LIBRARIES b)
#!   # RUNTIME=FALSE ENABLE_EXPORTS=FALSE LIBRARIES=a;NAME;n
#!   ex2 (LIBRARIES a NAME n)
#!   # error: unknown arguments NAME;n
#!   ex2 (NAME n LIBRARIES a)
#!   ex2 (LIBRARIES a ENABLE_EXPORTS NAME n)
#! ```
#!
#! \note Parsing can be slightly quirky, as seen in the last `ex2()`
#! call examples:
#! - `ex2()` does not have a `NAME` argument, so parsing of the
#!   `LIBRARIES` list won't stop there and `NAME` and `n` end up in
#!   the list.
#! - The second example is not currently parsing a list at the time it
#!   encounters `NAME n`, so it complains about unknown tokens.
#! - The third example also finished parsing the list and identifies
#!   `NAME n` as unknown tokens.
#! Similar, the `ex1 (NAME RUNTIME NAME b)` call stops searching for
#! the value of `NAME` at the `RUNTIME` token, leaves `_arg_NAME`
#! unset, but then sets it on second occurence.
#!
#! This quirk is most noticable when writing functions that use \see
#! _parse_arguments_with_unknown() to forward all unknown arguments to
#! a wrapped function. Such a function should avoid list arguments at
#! all cost as if they are followed by options unknown to the wrapping
#! function, they will be mixed in the list. For example
#! - `ex3()` accepting 1-value `NAME` and boolean `RUNTIME`
#! - `ex4()` accepting *-value `LIBRARIES` and forwarding unknown to
#!   `ex3()`
#! intends to have allow for `ex4 (NAME n LIBRARIES a b RUNTIME)`, but
#! `RUNTIME` will not reach `ex3()` as `LIBRARIES=a;b;RUNTIME` and
#! `UNPARSED=NAME;n`. `ex4 (NAME n RUNTIME LIBRARIES a b)` will
#! correctly have `LIBRARIES=a;b;` and `UNPARSED=NAME;n;RUNTIME`
#! though, which will lead to a very confused user.  An ugly
#! workaround is introducing a dummy boolean option and telling users
#! to end list arguments with it, i.e. `LIBRARIES a b END_LIBRARIES
#! RUNTIME`.
macro (_parse_arguments _prefix _options _one_value_options _multi_value_options _required_options)
  _parse_arguments_with_unknown ("${_prefix}" "${_options}" "${_one_value_options}" "${_multi_value_options}" "${_required_options}" ${ARGN})

  if (${_prefix}_UNPARSED_ARGUMENTS)
    list (LENGTH ${_prefix}_UNPARSED_ARGUMENTS _unparsed_length)
    if (NOT _unparsed_length EQUAL 0)
      message (FATAL_ERROR "unknown arguments: ${${_prefix}_UNPARSED_ARGUMENTS}")
    endif()
  endif()
endmacro()

#! Identical to \see _parse_arguments(), but instead of erroring on
#! unknown tokens it will populate `_prefix_UNPARSED_ARGUMENTS` as a
#! list of all tokens.
#!
#! \warn This function does not always behave as naturally
#! expected. Prefer to avoid using this function and carefully study
#! the note on parsing quirks in the \see _parse_arguments()
#! documentation.
#!
#! The order of unparsed tokens is preseved.
#!
#! ```
#!   function (ex5)
#!     _parse_arguments_with_unknown (_a "ENDLIST" "NAME" "LIST" "" ${ARGN})
#!     message (STATUS "NAME=${_a_NAME} LIST=${_a_LIST} unparsed=${_a_UNPARSED_ARGUMENTS}")
#!   endfunction()
#!   # NAME=a LIST= unparsed=
#!   ex5 (NAME a)
#!   # NAME=a LIST= unparsed=a
#!   ex5 (NAME a a)
#!   # NAME=a LIST= unparsed=a;a
#!   ex5 (a NAME a a)
#!   # NAME= LIST=a;unknown unparsed=
#!   ex5(LIST a unknown)
#!   # NAME= LIST=a unparsed=unknown
#!   ex5(LIST a ENDLIST unknown)
#! ```
macro (_parse_arguments_with_unknown _prefix _options _one_value_options _multi_value_options _required_options)
  cmake_parse_arguments ("${_prefix}" "${_options}" "${_one_value_options}" "${_multi_value_options}" ${ARGN})

  foreach (required ${_required_options})
    if (NOT ${_prefix}_${required})
      message (FATAL_ERROR "required argument ${required} missing")
    endif()
  endforeach()
endmacro()
