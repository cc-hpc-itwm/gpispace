// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      GSPC_DLLEXPORT value_type& poke
        ( std::list<std::string>::const_iterator const&
        , std::list<std::string>::const_iterator const&
        , value_type&
        , value_type const&
        );
      GSPC_DLLEXPORT value_type& poke
        ( std::list<std::string> const& path
        , value_type& node
        , value_type const& value
        );
      GSPC_DLLEXPORT value_type& poke
        (std::string const&, value_type&, value_type const&);
    }
  }
}
