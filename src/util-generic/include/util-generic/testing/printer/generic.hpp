// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
