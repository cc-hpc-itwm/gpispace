// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/value.hpp>

#include <list>
#include <string>

#include <boost/variant/recursive_wrapper_fwd.hpp>
#include <boost/variant/variant_fwd.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      using value_type = pnet::type::value::value_type;

      struct type;

      using path_type = std::list<std::string>;
    }
  }
}
