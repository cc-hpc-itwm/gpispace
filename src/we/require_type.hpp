// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/signature.hpp>
#include <we/type/value.hpp>

namespace pnet
{
  type::value::value_type const& require_type
    ( type::value::value_type const&
    , type::signature::signature_type const&
    );
  type::value::value_type const& require_type
    ( type::value::value_type const&
    , type::signature::signature_type const&
    , std::string const& field
    );
}
