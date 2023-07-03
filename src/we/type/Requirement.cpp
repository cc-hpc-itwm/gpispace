// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/Requirement.hpp>

namespace we
{
  namespace type
  {
    Requirement::Requirement (std::string value)
      : _value (value)
    {}

    std::string const& Requirement::value() const
    {
      return _value;
    }
  }
}
