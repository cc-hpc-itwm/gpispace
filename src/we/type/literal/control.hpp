// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <iosfwd>

namespace we
{
  namespace type
  {
    namespace literal
    {
      struct GSPC_DLLEXPORT control
      {
        GSPC_DLLEXPORT
          friend std::ostream& operator<< (std::ostream&, control const&);
        GSPC_DLLEXPORT
          friend bool operator== (control const&, control const&);

        GSPC_DLLEXPORT
          friend std::size_t hash_value (control const&);
        GSPC_DLLEXPORT
          friend bool operator< (control const&, control const&);

        template<typename Archive> void serialize (Archive&, unsigned int) {}
      };
    }
  }
}

//! \todo REMOVE! This is deprecated but some clients still use it.
using control = we::type::literal::control;
