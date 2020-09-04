// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#pragma once

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>

#include <string>

namespace we
{
  namespace type
  {
    struct requirement_t
    {
      requirement_t()
        : _value()
        , _mandatory (false)
      {}

      explicit
      requirement_t (std::string value, const bool mandatory = false)
        : _value (std::move (value))
        , _mandatory (mandatory)
      {}

      bool is_mandatory() const
      {
        return _mandatory;
      }

      std::string const& value() const
      {
        return _value;
      }

    private:
      friend class boost::serialization::access;

      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP (_value);
        ar & BOOST_SERIALIZATION_NVP (_mandatory);
      }

      std::string _value;
      bool _mandatory;
    };
  }
}
