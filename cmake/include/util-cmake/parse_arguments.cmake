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

include_guard()

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

###############################################################################
# Includes
###############################################################################

set (_include_dir ${CMAKE_CURRENT_LIST_DIR})

include (${_include_dir}/colors.cmake)

###############################################################################
# Private
###############################################################################

get_filename_component (_filename
  ${CMAKE_CURRENT_LIST_FILE}
  NAME_WLE
)

###############################################################################
# Public
###############################################################################

#[============================================================================[
### Description
`cmake_parse_arguments` wrapper with a more powerful and flexible argument
definition syntax.
It features:

- required arguments
- a flag to trigger an error message in case of unparsed arguments
- a flag to trigger an error message in case of keywords with missing values
- automatic generation of a usage message
- a flag to trigger the usage message in case the `HELP` argument is set
- default values
- appending of additional values to the default

#### Command:
```cmake
util_cmake_parse_arguments (
  <prefix>
  <arguments-to-parse>
  [<argument-type> [<argument-identifier>] [<argument-attributes>...]]...
)
```
`<argument-identifier>`s need to be valid CMake variable name string.
They can't be equivalent to any `<argument-type>` or `<argument-attribute>`
identifiers. Argument types and attributes are case insensitive!

#### Argument Types:
- ```
  OPTION <argument-identifier> [<argument-attribute>...]
  ```
  Defines a flag. Only the `DESCRIPTION` attribute makes sense for options.
  Other attributes are ignored and will issue a WARNING.
- ```
  SINGLE_VALUE <argument-identifier> [<argument-attribute>...]
  ```
  Defines an argument taking a single value.
- ```
  MULTI_VALUE <argument-identifier> [<argument-attribute>...]
  ```
  Defines an argument taking multiple values.
- ```
  ENABLE_HELP
  ```
  Flag to enables the `HELP` argument.
- ```
  NO_UNPARSED_ARGUMENTS
  ```
  Flag to enable the no unparsed arguments check.
- ```
  NO_MISSING_VALUES
  ```
  Flag to enable the no missing values check.

#### Argument Attributes:
- ```
  REQUIRED
  ```
  An error is triggered if the attributed argument is not present.
- ```
  DESCRIPTION <description-string>
  ```
  An arguments description string is used to populate the usage message.
- ```
  DEFAULT <value>...
  ```
  Default values for the attributed argument. `MULTI_VALUE` arguments may take
  multiple values in the form of a list.
- ```
  APPEND
  ```
  If this attribute and a default value are set, the default value is appended
  to the argument values. Duplicate values will be removed.
#]============================================================================]
function (util_cmake_parse_arguments
  _prefix
  _args
)
  set (_internal "${_prefix}_internal")
  set (_detail_dir "${_include_dir}/detail/${_filename}")
  set (_attributes_dir "${_detail_dir}/attributes")
  set (_types_dir "${_detail_dir}/types")

  # get available argument types
  file (GLOB
    _keywords
    LIST_DIRECTORIES false
    RELATIVE ${_types_dir}
    "${_types_dir}/*.cmake"
  )
  foreach (_keyword ${_keywords})
    get_filename_component (_keyword
      ${_keyword}
      NAME_WLE
    )
    list (APPEND ${_internal}_types ${_keyword})
  endforeach()

  # get available argument attributes
  file (GLOB
    _keywords
    LIST_DIRECTORIES false
    RELATIVE ${_attributes_dir}
    "${_attributes_dir}/*.cmake"
  )
  foreach (_keyword ${_keywords})
    get_filename_component (_keyword
      ${_keyword}
      NAME_WLE
    )
    list (APPEND ${_internal}_attributes ${_keyword})
  endforeach()

  # create argument definitions
  while (ARGN)
    list (POP_FRONT ARGN _type)
    string (TOLOWER "${_type}" _type)
    if (_type IN_LIST ${_internal}_types)
      include (${_types_dir}/${_type}.cmake)
    else()
      message (FATAL_ERROR
        "unknown argument type: ${_type} not in {${${_internal}_types}}"
      )
    endif()
  endwhile()

  # create required and optional argument strings
  set (_arg_column_size 30)
  foreach (_arg
    ${${_internal}_options}
    ${${_internal}_single_values}
    ${${_internal}_multi_values}
  )
    if (_arg IN_LIST ${_internal}_required)
      set (_usage_var "_required_args")
    else()
      set (_usage_var "_optional_args")
    endif()
    string (LENGTH "  ${_arg}" _length)
    string (APPEND ${_usage_var} "  ${_arg}")
    if (NOT _arg IN_LIST ${_internal}_options)
      string (APPEND ${_usage_var} " <value>")
      math (EXPR _length "${_length} + 8")
    endif()
    if (_arg IN_LIST ${_internal}_multi_values)
      string (APPEND ${_usage_var} "...")
      math (EXPR _length "${_length} + 3")
    endif()
    math (EXPR _length "${_arg_column_size} - ${_length}")
    if (${_internal}_${_arg}_description OR ${_internal}_${_arg}_default)
      if (_length LESS_EQUAL 0)
        string (APPEND ${_usage_var} "\n")
        set (_length ${_arg_column_size})
      endif()
      foreach (_space RANGE 1 ${_length})
        string (APPEND ${_usage_var} " ")
      endforeach()
      string (APPEND
        ${_usage_var}
        "${${_internal}_${_arg}_description}"
      )
      if (${_internal}_${_arg}_default)
        string (APPEND
          ${_usage_var}
          " (default: ${${_internal}_${_arg}_default})"
        )
      endif()
    endif()
    string (APPEND ${_usage_var} "\n")
  endforeach()

  # assemble usage string
  util_cmake_color_string (${_internal}_usage bold_blue "[[ USAGE ]]\n\n")
  util_cmake_color_string (_header bold_white "Required Arguments:")
  string (APPEND ${_internal}_usage "${_header}\n${_required_args}\n")
  util_cmake_color_string (_header bold_white "Optional Arguments:")
  string (APPEND ${_internal}_usage "${_header}\n${_optional_args}\n")

  # parse arguments
  cmake_parse_arguments (${_prefix}
    "${${_internal}_options}"
    "${${_internal}_single_values}"
    "${${_internal}_multi_values}"
    ${_args}
  )

  # verify no unparsed arguments
  if (${_internal}_no_unparsed_arguments AND ${_prefix}_UNPARSED_ARGUMENTS)
    message (STATUS "${${_internal}_usage}")
    message (FATAL_ERROR
      "Invalid arguments: ${${_prefix}_UNPARSED_ARGUMENTS}"
    )
  endif()

  # verify no missing arguments
  if (${_internal}_no_missing_values AND ${_prefix}_KEYWORDS_MISSING_VALUES)
    message (STATUS "${${_internal}_usage}")
    message (FATAL_ERROR
      "The following arguments are missing values: ${${_prefix}_KEYWORDS_MISSING_VALUES}"
    )
  endif()

  # verify required arguments
  if (${_internal}_required)
    foreach (_required ${${_internal}_required})
      if (NOT ${_prefix}_${_required})
        list (APPEND _missing ${_required})
      endif()
    endforeach()

    if (_missing)
      message (STATUS "${${_internal}_usage}")
      message (FATAL_ERROR
        "Missing required arguments: ${_missing}"
      )
    endif()
  endif()

  # check if help call was triggered
  if (${_internal}_enable_help AND ${_prefix}_HELP)
    message (STATUS "${${_internal}_usage}")
    message (FATAL_ERROR
      "Help message was triggered!"
    )
  endif()

  # return parsed results
  foreach (_arg
    ${${_internal}_options}
    ${${_internal}_single_values}
    ${${_internal}_multi_values}
  )
    if (${_prefix}_${_arg})
      if (${_internal}_${_arg}_append AND ${_internal}_${_arg}_default)
        list (APPEND ${_prefix}_${_arg} ${${_internal}_${_arg}_default})
        list (REMOVE_DUPLICATES ${_prefix}_${_arg})
      endif()
      set (${_prefix}_${_arg} ${${_prefix}_${_arg}} PARENT_SCOPE)
    elseif (${_internal}_${_arg}_default)
      set (${_prefix}_${_arg} ${${_internal}_${_arg}_default} PARENT_SCOPE)
    endif()
  endforeach()
  if (${_prefix}_UNPARSED_ARGUMENTS)
    set (${_prefix}_UNPARSED_ARGUMENTS ${${_prefix}_UNPARSED_ARGUMENTS} PARENT_SCOPE)
  endif()
endfunction()
