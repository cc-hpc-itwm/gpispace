// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/type/value.hpp>

#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/variant.hpp>

#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      GSPC_DLLEXPORT std::string to_string (value_type const&);
      GSPC_DLLEXPORT value_type from_string (std::string const&);
    }
  }
}
