// Copyright (C) 2013-2015,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/bitsetofint.hpp>
#include <gspc/we/type/bytearray.hpp>
#include <gspc/we/type/literal/control.hpp>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/variant.hpp>

#include <list>
#include <map>
#include <set>
#include <string>

namespace gspc::we::type
{
  class shared;
}


    namespace gspc::pnet::type::value
    {
      using bigint_type = boost::multiprecision::cpp_int;

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
        , pnet::type::bitsetofint::type
        , we::type::bytearray
        , bigint_type
        , we::type::shared
        , std::list<::boost::recursive_variant_>
        , std::set<::boost::recursive_variant_>
        , std::map<::boost::recursive_variant_, ::boost::recursive_variant_>
        , std::list<std::pair<std::string, ::boost::recursive_variant_>>
        >::type
        ;

      using structured_type = std::list<std::pair<std::string, value_type>>;
    }

// Include shared.hpp after value_type is defined (for shared.ipp)
#include <gspc/we/type/shared.hpp>
