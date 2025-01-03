# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

include (util-cmake/add_cxx_compiler_flag_if_supported)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED on)

add_cxx_compiler_flag_if_supported (-W)
add_cxx_compiler_flag_if_supported (-Wall)
add_cxx_compiler_flag_if_supported (-Wextra)
add_cxx_compiler_flag_if_supported (-Wbitfield-enum-conversion)
add_cxx_compiler_flag_if_supported (-Wc++20-compat)
add_cxx_compiler_flag_if_supported (-Wcomma)
add_cxx_compiler_flag_if_supported (-Wcovered-switch-default)
add_cxx_compiler_flag_if_supported (-Wdeprecated-copy-dtor)
add_cxx_compiler_flag_if_supported (-Wdeprecated-dynamic-exception-spec)
add_cxx_compiler_flag_if_supported (-Wextra-semi)
add_cxx_compiler_flag_if_supported (-Wextra-semi-stmt)
add_cxx_compiler_flag_if_supported (-Wfloat-equal)
add_cxx_compiler_flag_if_supported (-Wgnu-anonymous-struct)
add_cxx_compiler_flag_if_supported (-Wgnu-case-range)
add_cxx_compiler_flag_if_supported (-Wimplicit-fallthrough)
add_cxx_compiler_flag_if_supported (-Winconsistent-missing-destructor-override)
add_cxx_compiler_flag_if_supported (-Wmissing-prototypes)
add_cxx_compiler_flag_if_supported (-Wmissing-variable-declarations)
add_cxx_compiler_flag_if_supported (-Wnested-anon-types)
add_cxx_compiler_flag_if_supported (-Wno-redundant-move)
add_cxx_compiler_flag_if_supported (-Wnon-virtual-dtor)
add_cxx_compiler_flag_if_supported (-Wold-style-cast)
add_cxx_compiler_flag_if_supported (-Wpessimizing-move)
add_cxx_compiler_flag_if_supported (-Wrange-loop-analysis)
add_cxx_compiler_flag_if_supported (-Wreturn-std-move-in-c++11)
add_cxx_compiler_flag_if_supported (-Wshadow-field)
add_cxx_compiler_flag_if_supported (-Wshadow-uncaptured-local)
add_cxx_compiler_flag_if_supported (-Wundefined-func-template)
add_cxx_compiler_flag_if_supported (-Wunreachable-code)
add_cxx_compiler_flag_if_supported (-Wunreachable-code-break)
add_cxx_compiler_flag_if_supported (-Wunused)
add_cxx_compiler_flag_if_supported (-Wunused-macro)
add_cxx_compiler_flag_if_supported (-Wunused-exception-parameter)
add_cxx_compiler_flag_if_supported (-fPIC)
add_cxx_compiler_flag_if_supported (-fpic)
add_cxx_compiler_flag_if_supported (-fcolor-diagnostics)
add_cxx_compiler_flag_if_supported (-ftemplate-depth=1024)

# TODO: do not use deprecated declarations
add_cxx_compiler_flag_if_supported (-Wno-deprecated-declarations)

add_cxx_compiler_flag_if_supported (-Wno-maybe-uninitialized)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # do nothing
else()
  # Modern Debian has changed the default from `--no-as-needed` to
  # `--as-needed`, which leads to shared objects being filtered if the
  # linker can't prove it being used. This check fails when symbols are
  # needed only by dlopen()ed libraries, e.g. when linking
  # GPISpace::APIGuard to provide WE_GUARD_SYMBOL in drts-kernel. Thus,
  # be sure to always link without that optimization.
  add_cxx_compiler_flag_if_supported (-Wl,--no-as-needed)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -lstdc++fs")
  endif()
endif()
