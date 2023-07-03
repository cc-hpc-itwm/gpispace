// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/current_function.hpp>
#include <boost/format.hpp>
#include <stdexcept>
#define INVALID_ENUM_VALUE(type, value) \
  throw std::out_of_range \
     ((::boost::format \
      ("%1%:%2%: %3%: value of enum '%4%' expected: got non-enummed-value %5%") \
      % __FILE__ % __LINE__ % BOOST_CURRENT_FUNCTION % #type % value \
      ).str() \
     )
