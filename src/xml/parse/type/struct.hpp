// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/signature.hpp>

#include <list>
#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/variant.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct structure_type : with_position_of_definition
      {
      public:
        structure_type ( util::position_type const&
                       , pnet::type::signature::structured_type const& sig
                       );

        pnet::type::signature::structured_type const& signature() const;
        std::string const& name() const;

        void specialize (std::unordered_map<std::string, std::string> const&);
        void resolve (std::unordered_map<std::string, structure_type> const&);

      private:
        pnet::type::signature::structured_type _sig;
      };

      using structs_type = std::list<structure_type>;

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , structure_type const& st
                  );
      }
    }

    namespace structure_type_util
    {
      using set_type = std::unordered_map<std::string, type::structure_type>;
      using forbidden_type = std::unordered_map<std::string, std::string>;

      set_type make (type::structs_type const& structs, state::type const&);

      set_type join (set_type const& above, set_type const& below);
    }
  }
}
