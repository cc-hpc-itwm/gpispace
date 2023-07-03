// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/type/value.hpp>

#include <iosfwd>
#include <list>
#include <string>
#include <unordered_map>

namespace expr
{
  namespace eval
  {
    struct GSPC_DLLEXPORT context
    {
    private:
      std::unordered_map<std::string, pnet::type::value::value_type>
        _container;
      std::unordered_map<std::string, const pnet::type::value::value_type*>
        _ref_container;

      GSPC_DLLEXPORT
        friend std::ostream& operator<< (std::ostream&, context const&);

    public:
      void bind_ref (std::string const&, pnet::type::value::value_type const&);

      void bind_and_discard_ref ( std::list<std::string> const&
                                , pnet::type::value::value_type const&
                                );

      pnet::type::value::value_type const& value (std::list<std::string> const&) const;
    };
  }
}
