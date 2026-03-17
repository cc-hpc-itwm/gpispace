// Copyright (C) 2010-2016,2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/state.fwd.hpp>
#include <gspc/xml/parse/type/function.fwd.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/type_map_type.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <gspc/we/type/signature.hpp>

#include <list>
#include <string>
#include <unordered_map>

#include <boost/variant.hpp>

#include <filesystem>


  namespace gspc::xml::parse
  {
    namespace type
    {
      struct structure_type : with_position_of_definition
      {
      public:
        structure_type ( util::position_type const&
                       , gspc::pnet::type::signature::structured_type const& sig
                       );

        gspc::pnet::type::signature::structured_type const& signature() const;
        std::string const& name() const;

        void specialize (std::unordered_map<std::string, std::string> const&);
        void resolve (std::unordered_map<std::string, structure_type> const&);

      private:
        gspc::pnet::type::signature::structured_type _sig;
      };

      using structs_type = std::list<structure_type>;

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
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
