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

#include <xml/parse/util/property.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/warning.hpp>

#include <we/type/value/positions.hpp>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace property
      {
        namespace
        {
          void set ( state::type const& state
                   , we::type::property::type& prop
                   , we::type::property::path_type const& path
                   , we::type::property::value_type const& value
                   )
          {
            const ::boost::optional<pnet::type::value::value_type> old
              (prop.get (path));
            prop.set (path, value);

            if (old)
            {
              state.warn
                ( warning::property_overwritten ( path
                                                , *old
                                                , value
                                                , state.file_in_progress()
                                                )
                );
            }
          }
        }

        void set_state ( state::type& state
                       , we::type::property::type& prop
                       , we::type::property::path_type const& path
                       , we::type::property::value_type const& value
                       )
        {
          state.interpret_property (path, value);

          ::xml::parse::util::property::set (state, prop, path, value);
        }

        void join ( state::type const& state
                  , we::type::property::type& x
                  , we::type::property::type const& y
                  )
        {
          using path_and_value_type =
            std::pair<std::list<std::string>, pnet::type::value::value_type>;

          for ( path_and_value_type const& path_and_value
              : pnet::type::value::positions (y.value())
              )
          {
            ::xml::parse::util::property::set
              ( state
              , x
              , path_and_value.first
              , path_and_value.second
              );
          }
        }
      }
    }
  }
}
