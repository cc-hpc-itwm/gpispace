// Copyright (C) 2010,2013-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/state.hpp>

#include <gspc/we/type/property.hpp>




      namespace gspc::xml::parse::util::property
      {
        void set_state ( state::type& state
                       , ::gspc::we::type::property::type& prop
                       , ::gspc::we::type::property::path_type const& path
                       , ::gspc::we::type::property::value_type const& value
                       );
        void join ( state::type const& state
                  , ::gspc::we::type::property::type& x
                  , ::gspc::we::type::property::type const& y
                  );
      }
