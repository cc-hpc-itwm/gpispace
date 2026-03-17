// Copyright (C) 2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/Requirement.hpp>


  namespace gspc::we::type
  {
    Requirement::Requirement (std::string value)
      : _value (value)
    {}

    std::string const& Requirement::value() const
    {
      return _value;
    }
  }
