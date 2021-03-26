// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <boost/config.hpp>
#include <boost/version.hpp>

#if defined (BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)
  #define FHG_UTIL_CXX11_HAS_DEFAULTED_FUNCTIONS 1
#else
  #define FHG_UTIL_CXX11_HAS_DEFAULTED_FUNCTIONS 0
#endif

#if defined (__clang__) && BOOST_VERSION < 105700
  //! \note 4.4 introduced unique_ptr. the weird version detection is
  //! required as clang seems to break libstdc++ versions and is used
  //! in newer versions of boost.config since 1.57.0:
  //! https://github.com/boostorg/config/commit/b36566fe0
  #if defined (__GXX_EXPERIMENTAL_CXX0X__) && __has_include(<ratio>)
    #define FHG_UTIL_CXX11_HAS_UNIQUE_PTR 1
  #else
    #define FHG_UTIL_CXX11_HAS_UNIQUE_PTR 0
  #endif
#else
  #if !defined (BOOST_NO_CXX11_SMART_PTR)
    #define FHG_UTIL_CXX11_HAS_UNIQUE_PTR 1
  #else
    #define FHG_UTIL_CXX11_HAS_UNIQUE_PTR 0
  #endif
#endif
