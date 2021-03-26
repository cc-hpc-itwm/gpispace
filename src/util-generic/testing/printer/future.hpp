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

#include <util-generic/testing/printer/generic.hpp>

#include <future>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (std::future_status, os, status)
{
  switch (status)
  {
  case std::future_status::ready: os << "ready"; break;
  case std::future_status::timeout: os << "timeout"; break;
  case std::future_status::deferred: os << "deferred"; break;
  }
}
