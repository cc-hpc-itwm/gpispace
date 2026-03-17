// Copyright (C) 2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/signature/serialize.hpp>

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

SERIALIZE_EMPTY (::gspc::we::type::port::direction::In)
SERIALIZE_EMPTY (::gspc::we::type::port::direction::Out)
SERIALIZE_EMPTY (::gspc::we::type::port::direction::Tunnel)

#undef SERIALIZE_EMPTY


  namespace gspc::we::type
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
