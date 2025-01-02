// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fhg/util/parse/position.hpp>

#include <variant>

namespace fhg
{
  namespace util
  {
    unsigned long read_ulong (parse::position&);
    unsigned int read_uint (parse::position&);

    using num_type = std::variant< int
                                 , long
                                 , unsigned int
                                 , unsigned long
                                 , float
                                 , double
                                 >;

    num_type read_num (parse::position&);
  }
}
