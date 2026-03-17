// Copyright (C) 2012,2015,2020-2021,2023-2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/id.hpp>
#include <gspc/we/type/value.hpp>

#include <vector>


    namespace gspc::we::type
    {
      class Activity;

      struct TokenOnPort
      {
        pnet::type::value::value_type _token;
        we::port_id_type _port_id;

        template<typename Archive>
          auto serialize (Archive&, unsigned int) -> void;
      };

      using TokensOnPorts = std::vector<TokenOnPort>;
    }
