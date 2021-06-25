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

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/variant.hpp>

namespace we
{
  namespace type
  {
    template<class Archive>
      void activity_t::serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (_transition);
      ar & BOOST_SERIALIZATION_NVP (_transition_id);
      ar & BOOST_SERIALIZATION_NVP (_input);
      ar & BOOST_SERIALIZATION_NVP (_output);
      ar & BOOST_SERIALIZATION_NVP (_evaluation_context_requested);
      ar & BOOST_SERIALIZATION_NVP (_eureka_id);
    }
  }
}
