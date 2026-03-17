// Copyright (C) 2013-2015,2020-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/signature.hpp>

#include <optional>

#include <functional>
#include <string>



    namespace gspc::pnet::type::signature
    {
      using resolver_type = std::function< std::optional<signature_type> (const std::string &)>;

      signature_type resolve (structured_type const&, resolver_type const&);
    }
