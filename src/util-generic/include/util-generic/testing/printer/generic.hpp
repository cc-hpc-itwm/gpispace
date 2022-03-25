// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <boost/test/test_tools.hpp>

//! \note If this ever breaks, check BOOST_TEST_DONT_PRINT_LOG_VALUE.
#define FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER(TMPL, SPEC_PARAM, OS_NAME, VAL_NAME) \
  namespace boost                                                       \
  {                                                                     \
    namespace test_tools                                                \
    {                                                                   \
      namespace tt_detail                                               \
      {                                                                 \
        template TMPL struct print_log_value<SPEC_PARAM >               \
        {                                                               \
          void operator() (std::ostream&, const SPEC_PARAM&) const;     \
        };                                                              \
      }                                                                 \
    }                                                                   \
  }                                                                     \
  template TMPL                                                         \
  void ::boost::test_tools::tt_detail::print_log_value<SPEC_PARAM >::operator() \
    (std::ostream& OS_NAME, const SPEC_PARAM& VAL_NAME) const

#define FHG_BOOST_TEST_LOG_VALUE_PRINTER(SPEC_PARAM, OS_NAME, VAL_NAME) \
  namespace boost                                                       \
  {                                                                     \
    namespace test_tools                                                \
    {                                                                   \
      namespace tt_detail                                               \
      {                                                                 \
        template<> struct print_log_value<SPEC_PARAM >                  \
        {                                                               \
          void operator() (std::ostream&, const SPEC_PARAM&) const;     \
        };                                                              \
      }                                                                 \
    }                                                                   \
  }                                                                     \
  void ::boost::test_tools::tt_detail::print_log_value<SPEC_PARAM >::operator() \
    (std::ostream& OS_NAME, const SPEC_PARAM& VAL_NAME) const

#define FHG_BOOST_TEST_LOG_VALUE_PRINTER_WRAPPED(SPEC_PARAM, WRAPPER)   \
  FHG_BOOST_TEST_LOG_VALUE_PRINTER (SPEC_PARAM, os, val)                \
  {                                                                     \
    os << WRAPPER (val);                                                \
  }

#define FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER(...)            \
  ::boost::test_tools::tt_detail::print_helper (__VA_ARGS__)
