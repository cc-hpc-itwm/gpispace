// Copyright (C) 2013-2015,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/signature.hpp>

#include <string>
#include <unordered_set>



    namespace gspc::pnet::type::signature
    {
      std::unordered_set<std::string> names (signature_type const&);
    }
