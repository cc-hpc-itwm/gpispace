// Copyright (C) 2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/eureka.fwd.hpp>

#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <gspc/we/type/eureka.hpp>

#include <string>



    namespace gspc::xml::parse::type
    {
      struct eureka_type : with_position_of_definition
      {
      public:
        using unique_key_type = ::gspc::we::type::eureka_id_type;

        eureka_type ( util::position_type const&
                    , std::string const& port
                    );

        std::string const& port() const { return _port; }

        unique_key_type unique_key() const
        {
          return _port;
        }

      private:
        std::string const _port;
      };

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream&, eureka_type const&);
      }
    }
