// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/serialization/string.hpp>

namespace we
{
  namespace type
  {
    template<typename Archive>
      void Requirement::serialize (Archive& ar, unsigned int)
    {
      ar & _value;
    }
  }
}
