// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
      ::boost::serialization::split_free (ar, t, version);
    }
  }
}
