// Copyright (C) 2010,2014-2015,2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/hard_integral_typedef.hpp>

#include <string>


  namespace gspc::com
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
