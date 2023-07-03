// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fhg/util/parse/position.hpp>

#include <boost/variant.hpp>

namespace fhg
{
  namespace util
  {
    unsigned long read_ulong (parse::position&);
    unsigned int read_uint (parse::position&);

    using num_type = ::boost::variant< int
                                     , long
                                     , unsigned int
                                     , unsigned long
                                     , float
                                     , double
                                     >;

    num_type read_num (parse::position&);
  }
}
