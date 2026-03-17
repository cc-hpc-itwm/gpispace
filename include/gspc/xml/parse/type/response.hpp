// Copyright (C) 2015-2016,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/response.fwd.hpp>

#include <gspc/xml/parse/type/transition.fwd.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <gspc/we/type/property.hpp>


#include <string>



    namespace gspc::xml::parse::type
    {
      struct response_type : with_position_of_definition
      {
      public:
        //! \note port
        using unique_key_type = std::string;

        response_type ( util::position_type const&
                      , std::string const& port
                      , std::string const& to
                      , ::gspc::we::type::property::type const& = {}
                      );

        std::string const& port() const { return _port; }
        std::string const& to() const { return _to; }

        ::gspc::we::type::property::type const& properties() const
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
        ::gspc::we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream&, response_type const&);
      }
    }
