// Copyright (C) 2010-2013,2015-2016,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/expression.fwd.hpp>

#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/util/position.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <list>
#include <string>



    namespace gspc::xml::parse::type
    {
      //! \todo Move this into class scope.
      using expressions_type = std::list<std::string>;

      struct expression_type : with_position_of_definition
      {
      public:
        expression_type (util::position_type const&, expressions_type const&);

        std::string expression() const;

      private:
        expressions_type _expressions;
      };

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , expression_type const& e
                  );
      }
    }
