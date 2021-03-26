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

#pragma once

#include <boost/serialization/split_free.hpp>

#include <QtCore/QList>

namespace boost
{
  namespace serialization
  {
    template<class Archive, class U>
      inline void save (Archive& ar, QList<U> const& t, unsigned int const)
    {
      ar & t.size();
      for (U const& u : t)
      {
        ar & u;
      }
    }

    template<class Archive, class U>
      inline void load (Archive& ar, QList<U>& t, unsigned int const)
    {
      decltype (t.size()) size;
      ar & size;

      while (size --> 0)
      {
        U u;
        ar & u;
        t.push_back (u);
      }
    }

    template<class Archive, class U>
      inline void serialize (Archive& ar, QList<U>& t, unsigned int const version)
    {
      boost::serialization::split_free (ar, t, version);
    }
  }
}
