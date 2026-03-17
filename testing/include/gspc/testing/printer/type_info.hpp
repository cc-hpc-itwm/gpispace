// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/testing/printer/generic.hpp>

#include <typeinfo>

GSPC_BOOST_TEST_LOG_VALUE_PRINTER (std::type_info, os, info)
{
  os << info.name();
}
