// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#pragma once

#include <xml/parse/type/with_position_of_definition.hpp>

#include <xml/parse/util/position.fwd.hpp>

#include <we/type/property.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct memory_transfer_type : with_position_of_definition
      {
      public:
        memory_transfer_type ( const util::position_type&
                             , std::string const& global
                             , std::string const& local
                             , const we::type::property::type&
                             );

        std::string const& global() const
        {
          return _global;
        }
        std::string const& local() const
        {
          return _local;
        }
        const we::type::property::type& properties() const
        {
          return _properties;
        }

      private:
        std::string _global;
        std::string _local;
        we::type::property::type _properties;
      };

      struct memory_get : memory_transfer_type
      {
      public:
        memory_get ( const util::position_type&
                   , std::string const& global
                   , std::string const& local
                   , const we::type::property::type&
                   );
      };

      struct memory_put : memory_transfer_type
      {
      public:
        memory_put ( const util::position_type&
                   , std::string const& global
                   , std::string const& local
                   , const we::type::property::type&
                   , boost::optional<bool> const&
                   );
        boost::optional<bool> const& not_modified_in_module_call() const
        {
          return _not_modified_in_module_call;
        }

      private:
        boost::optional<bool> _not_modified_in_module_call;
      };

      struct memory_getput : memory_transfer_type
      {
      public:
        memory_getput ( const util::position_type&
                      , std::string const& global
                      , std::string const& local
                      , const we::type::property::type&
                      , boost::optional<bool> const&
                      );
        boost::optional<bool> const& not_modified_in_module_call() const
        {
          return _not_modified_in_module_call;
        }

      private:
        boost::optional<bool> _not_modified_in_module_call;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const memory_get&);
        void dump (::fhg::util::xml::xmlstream&, const memory_put&);
        void dump (::fhg::util::xml::xmlstream&, const memory_getput&);
      }
    }
  }
}
