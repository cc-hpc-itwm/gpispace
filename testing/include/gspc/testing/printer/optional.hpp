// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/testing/printer/generic.hpp>

#include <optional>
#include <functional>

GSPC_BOOST_TEST_LOG_VALUE_PRINTER (std::nullopt_t, os, )
{
  os << "Nothing";
}

GSPC_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename T>, std::optional<T>, os, opt)
{
  if (opt)
  {
    os << "Just " << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (*opt);
  }
  else
  {
    os << "Nothing";
  }
}

GSPC_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename T>, std::reference_wrapper<T>, os, ref)
{
  os << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (ref.get());
}
