// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/testing/printer/generic.hpp>

#include <boost/preprocessor/punctuation/comma.hpp>

#include <utility>

GSPC_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  ( <typename First BOOST_PP_COMMA() typename Second>
  , std::pair<First BOOST_PP_COMMA() Second>
  , os
  , p
  )
{
  os << "<"
     << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (std::get<0> (p))
     << ", "
     << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (std::get<1> (p))
     << ">";
}
