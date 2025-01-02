// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
