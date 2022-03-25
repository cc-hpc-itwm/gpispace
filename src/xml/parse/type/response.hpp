// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <xml/parse/type/response.fwd.hpp>

#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/property.hpp>

#include <boost/optional.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct response_type : with_position_of_definition
      {
      public:
        //! \note port
        using unique_key_type = std::string;

        response_type ( util::position_type const&
                      , std::string const& port
                      , std::string const& to
                      , we::type::property::type const& = {}
                      );

        std::string const& port() const { return _port; }
        std::string const& to() const { return _to; }

        we::type::property::type const& properties() const
        {
          return _properties;
        }

        unique_key_type unique_key() const
        {
          return _port;
        }

      private:
        std::string const _port;
        std::string _to;
        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, response_type const&);
      }
    }
  }
}
