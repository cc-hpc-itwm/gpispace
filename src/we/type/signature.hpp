// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/variant.hpp>

#include <list>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      using field_type = typename ::boost::make_recursive_variant
        < std::pair<std::string, std::string>
        , std::pair<std::string, std::list< ::boost::recursive_variant_>>
        >::type;

      using structure_type = std::list<field_type>;

      using structured_type = std::pair<std::string, structure_type>;

      using signature_type = ::boost::variant<std::string, structured_type>;
    }
  }
}
