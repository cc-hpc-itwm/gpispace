// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/type/value.hpp>

#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      class GSPC_DLLEXPORT show
      {
      public:
        show (value_type const&);
        std::ostream& operator() (std::ostream&) const;
      private:
        value_type const& _value;
      };
      GSPC_DLLEXPORT std::ostream& operator<< (std::ostream&, show const&);
    }
  }
}
