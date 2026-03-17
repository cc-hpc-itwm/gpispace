// Copyright (C) 2010,2013-2016,2019,2021-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/value.hpp>

#include <iosfwd>
#include <list>
#include <string>
#include <unordered_map>


  namespace gspc::we::expr::eval
  {
    struct GSPC_EXPORT context
    {
    private:
      std::unordered_map<std::string, pnet::type::value::value_type>
        _container;
      std::unordered_map<std::string, const pnet::type::value::value_type*>
        _ref_container;

      GSPC_EXPORT
        friend std::ostream& operator<< (std::ostream&, context const&);

    public:
      void bind_ref (std::string const&, pnet::type::value::value_type const&);

      void bind_and_discard_ref ( std::list<std::string> const&
                                , pnet::type::value::value_type const&
                                );

      pnet::type::value::value_type const& value (std::list<std::string> const&) const;
    };
  }
