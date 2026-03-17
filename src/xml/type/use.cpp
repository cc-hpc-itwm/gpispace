// Copyright (C) 2012-2013,2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/use.hpp>

#include <gspc/util/xml.hpp>



    namespace gspc::xml::parse::type
    {
      use_type::use_type ( util::position_type const& pod
                         , std::string const& name
                         )
        : with_position_of_definition (pod)
        , _name (name)
      {}

      std::string const& use_type::name() const
      {
        return _name;
      }

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream& s, use_type const& u)
        {
          s.open ("use");
          s.attr ("name", u.name());
          s.close ();
        }
      }
    }
