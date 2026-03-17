// Copyright (C) 2013,2015,2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/value.hpp>

#include <iosfwd>



    namespace gspc::pnet::type::value
    {
      class GSPC_EXPORT show
      {
      public:
        show (value_type const&);
        std::ostream& operator() (std::ostream&) const;
      private:
        value_type const& _value;
      };
      GSPC_EXPORT std::ostream& operator<< (std::ostream&, show const&);
    }
