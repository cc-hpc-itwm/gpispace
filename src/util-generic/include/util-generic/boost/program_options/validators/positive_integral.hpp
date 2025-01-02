// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_POSITIVE_INTEGRAL_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_POSITIVE_INTEGRAL_HPP

#include <util-generic/boost/program_options/validators/integral_greater_than.hpp>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        template<typename Base>
          using positive_integral = integral_greater_than<Base, 0>;
      }
    }
  }
}

#endif
