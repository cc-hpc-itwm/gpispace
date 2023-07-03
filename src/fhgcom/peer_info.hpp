// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/hard_integral_typedef.hpp>

#include <string>

namespace fhg
{
  namespace com
  {
    struct host_t
    {
      explicit host_t (std::string const&);
      operator std::string () const;

    private:
      std::string _hostname;
    };

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (port_t, unsigned short);
  }
}
