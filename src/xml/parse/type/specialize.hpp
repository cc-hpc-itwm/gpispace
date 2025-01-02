// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct specialize_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;

        specialize_type ( util::position_type const&
                        , std::string const& name
                        , std::string const& use
                        , type_map_type const& type_map
                        , type_get_type const& type_get
                        );

        std::string const& name () const;
        unique_key_type const& unique_key() const;

      private:
        std::string const _name;

        //! \todo All these should be private wth accessors.
      public:
        std::string use;
        type_map_type type_map;
        type_get_type type_get;
      };

      void split_structs ( parse::structure_type_util::set_type const& global
                         , structs_type & child_structs
                         , structs_type & parent_structs
                         , type_get_type const& type_get
                         , state::type const& state
                         );

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , specialize_type const& sp
                  );
      }
    }
  }
}
