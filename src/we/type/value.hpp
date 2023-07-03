// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/bitsetofint.hpp>
#include <we/type/bytearray.hpp>
#include <we/type/literal/control.hpp>

#include <boost/variant.hpp>

#include <list>
#include <map>
#include <set>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      using value_type = ::boost::make_recursive_variant
        < we::type::literal::control
        , bool
        , int
        , long
        , unsigned int
        , unsigned long
        , float
        , double
        , char
        , std::string
        , bitsetofint::type
        , we::type::bytearray
        , std::list<::boost::recursive_variant_>
        , std::set<::boost::recursive_variant_>
        , std::map<::boost::recursive_variant_, ::boost::recursive_variant_>
        , std::list<std::pair<std::string, ::boost::recursive_variant_>>
        >::type
        ;

      using structured_type = std::list<std::pair<std::string, value_type>>;
    }
  }
}
