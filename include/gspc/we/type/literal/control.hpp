// Copyright (C) 2013,2015,2021-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <iosfwd>



    namespace gspc::we::type::literal
    {
      struct GSPC_EXPORT control
      {
        GSPC_EXPORT
          friend std::ostream& operator<< (std::ostream&, control const&);
        GSPC_EXPORT
          friend bool operator== (control const&, control const&);

        GSPC_EXPORT
          friend std::size_t hash_value (control const&);
        GSPC_EXPORT
          friend bool operator< (control const&, control const&);

        template<typename Archive> void serialize (Archive&, unsigned int) {}
      };
    }



//! \todo REMOVE! This is deprecated but some clients still use it.
using control = gspc::we::type::literal::control;
