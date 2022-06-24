// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <we/type/property.fwd.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/serialize.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <boost/optional/optional_fwd.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>

#include <iosfwd>
#include <list>

namespace we
{
  namespace type
  {
    namespace property
    {
      struct type
      {
      public:
        type();

        value_type const& value() const;
        pnet::type::value::structured_type const& list() const;

        void set (path_type const& path, value_type const&);
        ::boost::optional<value_type const&> get (path_type const& path) const;
        bool is_true (path_type const&) const;

      private:
        value_type _value;

        friend class ::boost::serialization::access;
        template<typename Archive>
        void serialize (Archive& ar, unsigned int)
        {
          ar & _value;
        }
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, type const&);
      }

      std::ostream& operator<< (std::ostream& s, type const& t);
    }
  }
}
