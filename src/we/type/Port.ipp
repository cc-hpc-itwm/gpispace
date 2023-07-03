// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/signature/serialize.hpp>

#include <boost/serialization/optional.hpp>
#include <boost/serialization/variant.hpp>

#define SERIALIZE_EMPTY(D)                                \
  namespace boost                                         \
  {                                                       \
    namespace serialization                               \
    {                                                     \
      template<typename Archive>                          \
        void serialize (Archive&, D&, unsigned int)       \
      {}                                                  \
    }                                                     \
  }

SERIALIZE_EMPTY (::we::type::port::direction::In)
SERIALIZE_EMPTY (::we::type::port::direction::Out)
SERIALIZE_EMPTY (::we::type::port::direction::Tunnel)

#undef SERIALIZE_EMPTY

namespace we
{
  namespace type
  {
    template<typename Archive>
      void Port::serialize (Archive& ar, unsigned int)
    {
      ar & _name;
      ar & _direction;
      ar & _signature;
      ar & _associated_place;
      ar & _properties;
    }
  }
}
