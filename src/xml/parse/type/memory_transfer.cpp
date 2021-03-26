// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <xml/parse/type/memory_transfer.hpp>
#include <xml/parse/util/position.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      memory_transfer_type::memory_transfer_type
        ( const util::position_type& position_of_definition
        , std::string const& global
        , std::string const& local
        , const we::type::property::type& properties
        , boost::optional<bool> const& allow_empty_ranges
        )
          : with_position_of_definition (position_of_definition)
          , _global (global)
          , _local (local)
          , _properties (properties)
          , _allow_empty_ranges (allow_empty_ranges)
      {}

      boost::optional<bool> const&
        memory_transfer_type::allow_empty_ranges() const
      {
        return _allow_empty_ranges;
      }

      memory_get::memory_get
        ( const util::position_type& position_of_definition
        , std::string const& global
        , std::string const& local
        , const we::type::property::type& properties
        , boost::optional<bool> const& allow_empty_ranges
        )
          : memory_transfer_type
              ( position_of_definition
              , global
              , local
              , properties
              , allow_empty_ranges
              )
      {}

      memory_put::memory_put
        ( const util::position_type& position_of_definition
        , std::string const& global
        , std::string const& local
        , const we::type::property::type& properties
        , boost::optional<bool> const& not_modified_in_module_call
        , boost::optional<bool> const& allow_empty_ranges
        )
          : memory_transfer_type
              ( position_of_definition
              , global
              , local
              , properties
              , allow_empty_ranges
              )
          , _not_modified_in_module_call (not_modified_in_module_call)
      {}

      memory_getput::memory_getput
        ( const util::position_type& position_of_definition
        , std::string const& global
        , std::string const& local
        , const we::type::property::type& properties
        , boost::optional<bool> const& not_modified_in_module_call
        , boost::optional<bool> const& allow_empty_ranges
        )
          : memory_transfer_type
              ( position_of_definition
              , global
              , local
              , properties
              , allow_empty_ranges
              )
          , _not_modified_in_module_call (not_modified_in_module_call)
      {}

      namespace dump
      {
        namespace
        {
          void dump_transfer ( ::fhg::util::xml::xmlstream& s
                             , memory_transfer_type const& mt
                             )
          {
            s.attr ("allow-empty-ranges", mt.allow_empty_ranges());
            s.open ("global");
            s.content (mt.global());
            s.close();
            s.open ("local");
            s.content (mt.local());
            s.close();
          }
        }

        void dump (::fhg::util::xml::xmlstream& s, const memory_get& mg)
        {
          s.open ("memory-get");
          dump_transfer (s, mg);
          s.close();
        }
        void dump (::fhg::util::xml::xmlstream& s, const memory_put& mp)
        {
          s.open ("memory-put");
          s.attr
            ("not-modified-in-module-call", mp.not_modified_in_module_call());
          dump_transfer (s, mp);
          s.close();
        }
        void dump (::fhg::util::xml::xmlstream& s, const memory_getput& mp)
        {
          s.open ("memory-getput");
          s.attr
            ("not-modified-in-module-call", mp.not_modified_in_module_call());
          dump_transfer (s, mp);
          s.close();
        }
      }
    }
  }
}
