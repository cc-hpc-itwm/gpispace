// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/type/value.hpp>

#include <boost/optional.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      GSPC_DLLEXPORT ::boost::optional<value_type const&>
      peek ( std::list<std::string>::const_iterator const&
           , std::list<std::string>::const_iterator const&
           , value_type const&
           );
      GSPC_DLLEXPORT ::boost::optional<value_type const&>
      peek (std::list<std::string> const& path, value_type const& node);
      GSPC_DLLEXPORT ::boost::optional<value_type const&>
      peek (std::string const&, value_type const&);

      GSPC_DLLEXPORT ::boost::optional<value_type&>
      peek ( std::list<std::string>::const_iterator const&
           , std::list<std::string>::const_iterator const&
           , value_type&
           );
      GSPC_DLLEXPORT ::boost::optional<value_type&>
      peek (std::list<std::string> const& path, value_type& node);
      GSPC_DLLEXPORT ::boost::optional<value_type&>
      peek (std::string const&, value_type&);
    }
  }
}
