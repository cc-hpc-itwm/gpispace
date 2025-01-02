// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/serialization/optional.hpp>
#include <boost/serialization/variant.hpp>

namespace we
{
  namespace type
  {
    template<class Archive>
      void Activity::serialize (Archive& ar, unsigned int)
    {
      ar & _transition;
      ar & _transition_id;
      ar & _input;
      ar & _output;
      ar & _evaluation_context_requested;
      ar & _eureka_id;
    }
  }
}

namespace we::type
{
  template<typename Archive>
    auto TokenOnPort::serialize (Archive& ar, unsigned int) -> void
  {
    ar & _token & _port_id;
  }
}
