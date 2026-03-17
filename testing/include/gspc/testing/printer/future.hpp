// Copyright (C) 2017,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/testing/printer/generic.hpp>

#include <future>

GSPC_BOOST_TEST_LOG_VALUE_PRINTER (std::future_status, os, status)
{
  switch (status)
  {
  case std::future_status::ready: os << "ready"; break;
  case std::future_status::timeout: os << "timeout"; break;
  case std::future_status::deferred: os << "deferred"; break;
  }
}
