// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
