// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/specialize.hpp>

#include <xml/parse/type/net.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      specialize_type::specialize_type ( util::position_type const& pod
                                       , std::string const& name
                                       , std::string const& use_
                                       , type_map_type const& type_map_
                                       , type_get_type const& type_get_
                                       )
        : with_position_of_definition (pod)
        , _name (name)
        , use (use_)
        , type_map (type_map_)
        , type_get (type_get_)
      {}

      std::string const& specialize_type::name() const
      {
        return _name;
      }

      specialize_type::unique_key_type const&
        specialize_type::unique_key() const
      {
        return name();
      }

      void split_structs ( xml::parse::structure_type_util::set_type const& global
                         , structs_type & child_structs
                         , structs_type & parent_structs
                         , type_get_type const& type_get
                         , state::type const& state
                         )
      {
        namespace st = xml::parse::structure_type_util;

        const st::set_type known_structs
          ( st::join ( global
                     , st::join ( st::make (parent_structs, state)
                                , st::make (child_structs, state)
                                )
                     )
          );

        structs_type structs (child_structs);

        child_structs.clear();

        for (structure_type& s : structs)
        {
          const type_get_type::const_iterator pos (type_get.find (s.name()));

          if (pos == type_get.end())
          {
            child_structs.push_back (s);
          }
          else
          {
            s.resolve (known_structs);

            parent_structs.push_back (s);
          }
        }
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , specialize_type const& sp
                  )
        {
          s.open ("specialize");
          s.attr ("name", sp.name());
          s.attr ("use", sp.use);

          for (auto const& tm : sp.type_map)
          {
            s.open ("type-map");
            s.attr ("replace", tm.first);
            s.attr ("with", tm.second);
            s.close ();
          }

          for (auto const& tg : sp.type_get)
          {
            s.open ("type-get");
            s.attr ("name", tg);
            s.close ();
          }

          s.close ();
        }
      }
    }
  }
}
