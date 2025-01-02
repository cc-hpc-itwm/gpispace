// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
