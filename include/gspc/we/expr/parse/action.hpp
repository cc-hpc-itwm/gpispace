// Copyright (C) 2010,2012-2013,2015,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/token/type.hpp>

#include <iosfwd>



    namespace gspc::we::expr::parse::action
    {
      enum type
      { shift
      , reduce
      , accept
      , error1
      , error2
      , error3
      , error4
      };

      std::ostream& operator<< (std::ostream&, type const&);
      type action (token::type const&, token::type const&);
    }
