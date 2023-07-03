// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/printer/generic.hpp>

#include <boost/optional.hpp>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (::boost::none_t, os, )
{
  os << "Nothing";
}

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename T>, ::boost::optional<T>, os, opt)
{
  if (opt)
  {
    os << "Just " << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (opt.get());
  }
  else
  {
    os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (::boost::none);
  }
}
