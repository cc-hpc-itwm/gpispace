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

include (util-cmake/add_cxx_compiler_flag_if_supported)

set (CMAKE_CXX_STANDARD 14)
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

add_cxx_compiler_flag_if_supported (-Wno-maybe-uninitialized)

# Modern Debian has changed the default from `--no-as-needed` to
# `--as-needed`, which leads to shared objects being filtered if the
# linker can't prove it being used. This check fails when symbols are
# needed only by dlopen()ed libraries, e.g. when linking
# GPISpace::APIGuard to provide WE_GUARD_SYMBOL in drts-kernel. Thus,
# be sure to always link without that optimization.
add_cxx_compiler_flag_if_supported (-Wl,--no-as-needed)
