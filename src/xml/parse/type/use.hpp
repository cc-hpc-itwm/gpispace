// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct use_type : with_position_of_definition
      {
      public:
        use_type ( util::position_type const&
                 , std::string const& name
                 );

        std::string const& name() const;

      private:
        std::string _name;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, use_type const& u);
      }
    }
  }
}
