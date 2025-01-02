// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/serialization/access.hpp>

#include <string>

namespace we
{
  namespace type
  {
    struct Requirement
    {
      explicit Requirement (std::string value);

      std::string const& value() const;

      Requirement() = default; //! \note serialization only

    private:
      std::string _value;

      friend class ::boost::serialization::access;
      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };
  }
}

#include <we/type/Requirement.ipp>
