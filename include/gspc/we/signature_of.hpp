// Copyright (C) 2013,2015,2021-2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/signature.hpp>
#include <gspc/we/type/value.hpp>

namespace gspc::pnet
{
  GSPC_EXPORT type::signature::signature_type
    signature_of (type::value::value_type const&);
}
