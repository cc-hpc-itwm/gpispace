// Copyright (C) 2010-2013,2015-2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/transition.fwd.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <string>



    namespace gspc::xml::parse::type
    {
      struct use_type : with_position_of_definition
      {
      public:
        use_type ( util::position_type const&
                 , std::string const& name
                 );

        std::string const& name() const;

      private:
        std::string _name;
      };

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream& s, use_type const& u);
      }
    }
