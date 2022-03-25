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

#"BEGIN GUARD"
include_guard()
#"END GUARD"

#"BEGIN INCLUDE"
include (util-cmake/parse_arguments)
#"END INCLUDE"

# determine location of util-cmake scripts
set (_modules_dir "${CMAKE_CURRENT_LIST_DIR}/../../modules")
set (_include_dir "${CMAKE_CURRENT_LIST_DIR}")

#[[
### Description:
This function is the implementation of `util_cmake_scripts (FIND ...)`.
It locates util-cmake scripts and returns their absolute paths.
If one of the requested files can't be found, the function throws a fatal error.

#### Command:
```cmake
_util_cmake_find_scripts (
  NAMES <script-names>
  OUTPUT <output-variable>
  [APPEND]
)
```

#### Arguments:
- NAMES:  Names of the scripts to find
- OUTPUT: Variable to output the result into
- APPEND: Flag, that if set appends the output to the given output variable

Required: NAMES, OUTPUT
#]]
function (_util_cmake_find_scripts)
  set (_options APPEND)
  set (_one_value_options OUTPUT)
  set (_multi_value_options NAMES)
  set (_required_options NAMES OUTPUT)
  _parse_arguments (ARG "${_options}" "${_one_value_options}" "${_multi_value_options}" "${_required_options}" ${ARGN})

  set (_script_path)
  foreach(_script_name ${ARG_NAMES})
    get_filename_component (_script_name "${_script_name}" NAME_WE)

    if (EXISTS "${_modules_dir}/${_script_name}.cmake")
      list (APPEND _script_path "${_modules_dir}/${_script_name}.cmake")
    elseif (EXISTS "${_include_dir}/${_script_name}.cmake")
      list (APPEND _script_path "${_include_dir}/${_script_name}.cmake")
    else ()
      message (FATAL_ERROR "Couldn't find script ${_script_name}!")
    endif ()
  endforeach()

  if (ARG_APPEND)
    list (APPEND ${ARG_OUTPUT} "${_script_path}")
  else ()
    set (${ARG_OUTPUT} "${_script_path}")
  endif ()
  set (${ARG_OUTPUT} ${${ARG_OUTPUT}} PARENT_SCOPE)
endfunction ()

#[[
### Description:
This function is the implementation of `util_cmake_scripts (DEPENDENCIES ...)`.
It gathers all the dependencies of the given util-cmake scripts and returns a list with their absolute paths.
Scripts which are not a part of util-cmake will cause an error.

#### Command:
```cmake
_util_cmake_dependencies_scripts (
  NAMES <script-names>
  OUTPUT <output-variable>
  [APPEND]
)
```

#### Arguments:
- NAMES:  Names of the scripts to read
- OUTPUT: Variable to output the concatenated result into
- APPEND: Flag, that if set appends the output to the given output variable

Required: NAMES, OUTPUT
#]]
function (_util_cmake_dependencies_scripts)
  set (_options APPEND)
  set (_one_value_options OUTPUT)
  set (_multi_value_options NAMES)
  set (_required_options NAMES OUTPUT)
  _parse_arguments (ARG "${_options}" "${_one_value_options}" "${_multi_value_options}" "${_required_options}" ${ARGN})

  # init output
  # this step adds the absolute path to each input
  _util_cmake_find_scripts (
    NAMES ${ARG_NAMES}
    OUTPUT _previous
  )

  while (NOT _current_size EQUAL _previous_size)
    set (_current)
    foreach (_item ${_previous})
      # includes
      if (_item STREQUAL "${_include_dir}/_bundle.cmake")
        list (APPEND _current
          "${_include_dir}/bundle.sh"
          "${_include_dir}/install_directory.cmake"
        )
      elseif (_item STREQUAL "${_include_dir}/_test_wrapper.cmake")
      elseif (_item STREQUAL "${_include_dir}/add_cxx_compiler_flag_if_supported.cmake")
        list (APPEND _current
          "${_include_dir}/parse_arguments.cmake"
        )
      elseif (_item STREQUAL "${_include_dir}/add_macros.cmake")
        list (APPEND _current
          "${_include_dir}/_bundle.cmake"
          "${_include_dir}/add_cxx_compiler_flag_if_supported.cmake"
          "${_include_dir}/parse_arguments.cmake"
        )
      elseif (_item STREQUAL "${_include_dir}/add_unit_test.cmake")
        list (APPEND _current
          "${_include_dir}/parse_arguments.cmake"
          "${_include_dir}/add_macros.cmake"
          "${_include_dir}/beautify_find_boost.cmake"
          "${_include_dir}/_test_wrapper.cmake"
        )
      elseif (_item STREQUAL "${_include_dir}/beautify_find_boost.cmake")
        list (APPEND _current
          "${_include_dir}/add_macros.cmake"
          "${_include_dir}/parse_arguments.cmake"
          "${_include_dir}/install_directory.cmake"
        )
      elseif (_item STREQUAL "${_include_dir}/beautify_find_GPISpace.cmake")
        list (APPEND _current
          "${_include_dir}/parse_arguments.cmake"
        )
      elseif (_item STREQUAL "${_include_dir}/bundle.sh")
      elseif (_item STREQUAL "${_include_dir}/doxygen.cmake")
        list (APPEND _current
          "${_include_dir}/parse_arguments.cmake"
          "${_include_dir}/install_directory.cmake"
        )
      elseif (_item STREQUAL "${_include_dir}/git_submodules.cmake")
      elseif (_item STREQUAL "${_include_dir}/git.cmake")
      elseif (_item STREQUAL "${_include_dir}/install_directory.cmake")
      elseif (_item STREQUAL "${_include_dir}/parse_arguments.cmake")
      elseif (_item STREQUAL "${_include_dir}/projects.cmake")
      elseif (_item STREQUAL "${_include_dir}/require_compiler_version.cmake")
        list (APPEND _current
          "${_include_dir}/parse_arguments.cmake"
        )
      elseif (_item STREQUAL "${_include_dir}/scripts.cmake")
        list (APPEND _current
          "${_include_dir}/parse_arguments.cmake"
        )

      # modules
      elseif (_item STREQUAL "${_modules_dir}/FindASCIIDOC.cmake")
        list (APPEND _current
          "${_include_dir}/parse_arguments.cmake"
        )
      elseif (_item STREQUAL "${_modules_dir}/FindGASPI.cmake")
        list (APPEND _current
          "${_modules_dir}/detail/pkgconfig_helper.cmake"
        )
      elseif (_item STREQUAL "${_modules_dir}/FindHWLOC.cmake")
        list (APPEND _current
          "${_modules_dir}/detail/pkgconfig_helper.cmake"
        )
      elseif (_item STREQUAL "${_modules_dir}/Findlibfuse.cmake")
        list (APPEND _current
          "${_modules_dir}/detail/pkgconfig_helper.cmake"
        )
      elseif (_item STREQUAL "${_modules_dir}/FindLibssh2.cmake")
        list (APPEND _current
          "${_modules_dir}/detail/pkgconfig_helper.cmake"
        )
      elseif (_item STREQUAL "${_modules_dir}/FindSourceHighlight.cmake")
        list (APPEND _current
          "${_modules_dir}/detail/pkgconfig_helper.cmake"
        )

      # modules/detail
      elseif (_item STREQUAL "${_modules_dir}/detail/pkgconfig_helper.cmake")
      endif ()
    endforeach ()

    # append output from previous iteration
    list (APPEND _current ${_previous})

    # eliminate duplicates (NOTE: first occurence remains)
    list (REMOVE_DUPLICATES _current)

    # iteration finishes if the size of the current iteration and the last iteration is the same
    list (LENGTH _current _current_size)
    list (LENGTH _previous _previous_size)

    # override previous with current
    set (_previous ${_current})
  endwhile ()

  # output
  if (ARG_APPEND)
    list (APPEND ${ARG_OUTPUT} "${_previous}")
  else ()
    set (${ARG_OUTPUT} "${_previous}")
  endif ()
  set (${ARG_OUTPUT} ${${ARG_OUTPUT}} PARENT_SCOPE)
endfunction ()

#[[
### Description:
This function is the implementation of `util_cmake_scripts (NEEDS_INSTALL ...)`.
It takes a list of absolute paths of util-cmake scripts and returns a list of the ones that require installation and can't be inlined.
This result also includes the listed scripts' dependencies.
Scripts which are not a part of util-cmake will cause an error.

#### Command:
```cmake
_util_cmake_needs_install_scripts (
  NAMES <script-paths>
  OUTPUT <output-variable>
  [APPEND]
)
```

#### Arguments:
- NAMES:  Absolute paths of the scripts to verify
- OUTPUT: Variable to output the concatenated result into
- APPEND: Flag, that if set appends the output to the given output variable

Required: NAMES, OUTPUT
#]]
function (_util_cmake_needs_install_scripts)
  set (_options APPEND)
  set (_one_value_options OUTPUT)
  set (_multi_value_options NAMES)
  set (_required_options NAMES OUTPUT)
  _parse_arguments (ARG "${_options}" "${_one_value_options}" "${_multi_value_options}" "${_required_options}" ${ARGN})

  # get the entire dependencies list for the input scripts
  _util_cmake_dependencies_scripts (
    NAMES ${ARG_NAMES}
    OUTPUT _scripts
  )

  # this checklist needs manual updating
  # it contains all util-cmake scripts that can't be inlined
  # CMake Find modules generally can't be inlined either
  # not being able to be inlined means the script is not explicitely included (e.g. include (util-cmake/add_macros))
  set (_needs_install
    "${_include_dir}/bundle.sh"
    "${_include_dir}/_test_wrapper.cmake"
    "${_modules_dir}/FindASCIIDOC.cmake"
    "${_modules_dir}/FindGASPI.cmake"
    "${_modules_dir}/FindHWLOC.cmake"
    "${_modules_dir}/Findlibfuse.cmake"
    "${_modules_dir}/FindLibssh2.cmake"
    "${_modules_dir}/FindSourceHighlight.cmake"
  )

  foreach (_item ${_scripts})
    if (_item IN_LIST _needs_install)
      list (APPEND _output "${_item}")
    endif ()
  endforeach ()

  # output
  if (ARG_APPEND)
    list (APPEND ${ARG_OUTPUT} "${_output}")
  else ()
    set (${ARG_OUTPUT} "${_output}")
  endif ()
  set (${ARG_OUTPUT} ${${ARG_OUTPUT}} PARENT_SCOPE)
endfunction ()

#[[
### Description:
This function is the implementation of `util_cmake_scripts (INLINE ...)`.
It creates a dependencies chain list, reads the contents each file, strips the content of inlcude guards and marked include blocks, and outputs the concatenated stripped content.
Scripts which are not a part of util-cmake will cause an error.

#### Command:
```cmake
_util_cmake_inline_scripts (
  NAMES <script-names>
  OUTPUT <output-variable>
)
```

#### Arguments:
- NAMES:  Names of the scripts to read
- OUTPUT: Variable to output the concatenated result into

Required: NAMES, OUTPUT
#]]
function (_util_cmake_inline_scripts)
  set (_options)
  set (_one_value_options OUTPUT)
  set (_multi_value_options NAMES)
  set (_required_options NAMES OUTPUT)
  _parse_arguments (ARG "${_options}" "${_one_value_options}" "${_multi_value_options}" "${_required_options}" ${ARGN})

  _util_cmake_dependencies_scripts (
    NAMES ${ARG_NAMES}
    OUTPUT _script_paths
  )

  _util_cmake_needs_install_scripts (
    NAMES ${_script_paths}
    OUTPUT _needs_install
  )

  set (${ARG_OUTPUT})
  foreach (_script_path ${_script_paths})
    if (_script_path IN_LIST _needs_install)
      message (FATAL_ERROR "util_cmake_scripts: ${_script_path} can't be inlined! INLINE called for: ${ARG_NAMES}")
    endif ()

    file (READ "${_script_path}" _script_sources)

    # remove marked blocks
    # there can only be one of each kind per script, since regex can't properly match open-close symbol pairs with arbitrary content in-between
    # if begin and end symbols without escaped characters are chosen, executing this function on this script would corrupt the output
    # besides those restrictions, any text can be inserted (e.g. #\\[\\[GUARD\\]\\] ... #\\[\\[END GUARD\\]\\])
    # those markers are only recommended for util-cmake scripts
    set (_guard_block "#\"BEGIN GUARD\".*#\"END GUARD\"")
    set (_include_block "#\"BEGIN INCLUDE\".*#\"END INCLUDE\"")
    string (REGEX REPLACE
      "(${_guard_block})|(${_include_block})"
      ""
      _script_sources
      "${_script_sources}"
    )

    string (APPEND ${ARG_OUTPUT} "${_script_sources}")
  endforeach ()

  set (${ARG_OUTPUT} "${${ARG_OUTPUT}}" PARENT_SCOPE)
endfunction ()

#[[
### Description:
This function allows agnostic interactions with util-cmake scripts and should be preferred over direct access for stability reasons.
Currently the following modes are supported:
- DEPENDENCIES
- INLINE

#### Command:
```cmake
util_cmake_scripts (
  <mode>
  [<arg>...]
)
```

#### Arguments:
- <mode>: Selects the script operation to be executed
#]]
function (util_cmake_scripts _mode)
  set (_options)
  set (_one_value_options OUTPUT)
  set (_multi_value_options)
  set (_required_options)
  _parse_arguments_with_unknown (ARG "${_options}" "${_one_value_options}" "${_multi_value_options}" "${_required_options}" ${ARGN})

  if (_mode STREQUAL "DEPENDENCIES")
    _util_cmake_dependencies_scripts (${ARGN})
  elseif (_mode STREQUAL "INLINE")
    _util_cmake_inline_scripts (${ARGN})
  else ()
    message (FATAL_ERROR "util_cmake_scripts: Unknown mode ${_mode}! Supported modes are: DEPENDENCIES, INLINE")
  endif ()

  if (ARG_OUTPUT)
    set (${ARG_OUTPUT} ${${ARG_OUTPUT}} PARENT_SCOPE)
  endif ()
endfunction ()
