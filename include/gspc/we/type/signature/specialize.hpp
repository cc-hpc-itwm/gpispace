// Copyright (C) 2013-2015,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/signature.hpp>

#include <string>
#include <unordered_map>



    namespace gspc::pnet::type::signature
    {
      void specialize ( structured_type&
                      , std::unordered_map<std::string, std::string> const&
                      );
    }
