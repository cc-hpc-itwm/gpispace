// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/QString>

#include <boost/serialization/split_free.hpp>

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load (Archive& ar, QString& string, unsigned int const)
    {
      std::string std_string;

      ar & std_string;

      string = QString::fromStdString (std_string);
    }

    template<typename Archive>
      void save (Archive& ar, QString const& string, unsigned int const)
    {
      ar & string.toStdString();
    }

    template<class Archive>
      void serialize (Archive& ar, QString& string, unsigned int const version)
    {
      ::boost::serialization::split_free (ar, string, version);
    }
  }
}
