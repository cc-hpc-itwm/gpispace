// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/type/signature/serialize.hpp>

#include <boost/serialization/nvp.hpp>
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
      ar & BOOST_SERIALIZATION_NVP (_name);
      ar & BOOST_SERIALIZATION_NVP (_direction);
      ar & BOOST_SERIALIZATION_NVP (_signature);
      ar & BOOST_SERIALIZATION_NVP (_associated_place);
      ar & BOOST_SERIALIZATION_NVP (_properties);
    }
  }
}
