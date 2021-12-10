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
