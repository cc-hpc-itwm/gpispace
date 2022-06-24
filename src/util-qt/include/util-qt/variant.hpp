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

#include <boost/optional.hpp>

#include <QtCore/QMetaType>
#include <QtCore/QVariant>

#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      template<typename T> bool stores (QVariant const& variant)
      {
        return qMetaTypeId<T>() == variant.userType();
      }

      template<typename T> T value (QVariant const& variant)
      {
        if (!stores<T> (variant))
        {
          throw std::invalid_argument
            ( "value<T> (variant): QVariant contained wrong type: expected="
            + std::string (QMetaType::typeName (qMetaTypeId<T>())) + " got="
            + std::string (variant.typeName())
            );
        }

        return variant.value<T>();
      }

      template<typename T> ::boost::optional<T> optional (QVariant const& variant)
      {
        return stores<T> (variant)
          ? ::boost::optional<T> (variant.value<T>())
          : ::boost::optional<T> (::boost::none);
      }

      template<typename T> QList<T> collect (QVariant const& variant)
      {
        QList<T> result;

        if (stores<T> (variant))
        {
          result << variant.value<T>();
        }
        else if (stores<QVariantList> (variant))
        {
          for (QVariant const& v : variant.toList())
          {
            result << collect<T> (v);
          }
        }

        return result;
      }
    }
  }
}
