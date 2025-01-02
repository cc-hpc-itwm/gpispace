// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/type/value.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      class position;
    }
  }
}

namespace pnet
{
  namespace type
  {
    namespace value
    {
      GSPC_DLLEXPORT value_type read (fhg::util::parse::position&);
      GSPC_DLLEXPORT value_type read (std::string const&);
    }
  }
}
