// Copyright (C) 2013-2014,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/property.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/warning.hpp>

#include <gspc/we/type/value/positions.hpp>

#include <optional>




      namespace gspc::xml::parse::util::property
      {
        namespace
        {
          void set ( state::type const& state
                   , ::gspc::we::type::property::type& prop
                   , ::gspc::we::type::property::path_type const& path
                   , ::gspc::we::type::property::value_type const& value
                   )
          {
            auto const old = prop.get (path);
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
                       , ::gspc::we::type::property::type& prop
                       , ::gspc::we::type::property::path_type const& path
                       , ::gspc::we::type::property::value_type const& value
                       )
        {
          state.interpret_property (path, value);

          ::gspc::xml::parse::util::property::set (state, prop, path, value);
        }

        void join ( state::type const& state
                  , ::gspc::we::type::property::type& x
                  , ::gspc::we::type::property::type const& y
                  )
        {
          using path_and_value_type =
            std::pair<std::list<std::string>, gspc::pnet::type::value::value_type>;

          for ( path_and_value_type const& path_and_value
              : gspc::pnet::type::value::positions (y.value())
              )
          {
            ::gspc::xml::parse::util::property::set
              ( state
              , x
              , path_and_value.first
              , path_and_value.second
              );
          }
        }
      }
