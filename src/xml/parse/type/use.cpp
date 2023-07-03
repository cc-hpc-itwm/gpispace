// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/use.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
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
        void dump (::fhg::util::xml::xmlstream& s, use_type const& u)
        {
          s.open ("use");
          s.attr ("name", u.name());
          s.close ();
        }
      }
    }
  }
}
