// Copyright (C) 2013,2015,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/parse/position.hpp>

#include <boost/multiprecision/cpp_int.hpp>

#include <variant>


  namespace gspc::util
  {
    using bigint = boost::multiprecision::cpp_int;

    unsigned long read_ulong (parse::position&);
    unsigned int read_uint (parse::position&);

    using num_type = std::variant< int
                                 , long
                                 , unsigned int
                                 , unsigned long
                                 , float
                                 , double
                                 , bigint
                                 >;

    num_type read_num (parse::position&);
  }
