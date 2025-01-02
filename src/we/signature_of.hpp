// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/type/signature.hpp>
#include <we/type/value.hpp>

namespace pnet
{
  GSPC_DLLEXPORT type::signature::signature_type
    signature_of (type::value::value_type const&);
}
