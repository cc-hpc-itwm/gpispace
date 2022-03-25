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

#include <boost/config.hpp>

#include <boost/serialization/collection_traits.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/serialization/collections_save_imp.hpp>
#include <boost/serialization/split_free.hpp>

#include <QtCore/QVector>

namespace boost
{
  namespace serialization
  {
    template<class Archive, class U>
      inline void save (Archive& ar, QVector<U> const& t, unsigned int const)
    {
      ::boost::serialization::stl::save_collection<Archive, QVector<U>> (ar, t);
    }

    template<class Archive, class U>
      inline void load (Archive& ar, QVector<U>& t, unsigned int const)
    {
      ::boost::serialization::stl::load_collection
        < Archive
        , QVector<U>
        , ::boost::serialization::stl::archive_input_seq<Archive, QVector<U>>
        , ::boost::serialization::stl::no_reserve_imp<QVector<U>>
        > (ar, t);
    }

    template<class Archive, class U>
      inline void serialize (Archive& ar, QVector<U>& t, unsigned int const version)
    {
      ::boost::serialization::split_free (ar, t, version);
    }
  }
}

BOOST_SERIALIZATION_COLLECTION_TRAITS (QVector)
