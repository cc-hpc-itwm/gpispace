#pragma once

#include <optional>

#include <QtCore/QMetaType>
#include <QtCore/QVariant>

#include <stdexcept>
#include <string>



    namespace gspc::util::qt
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

      template<typename T> std::optional<T> optional (QVariant const& variant)
      {
        return stores<T> (variant)
          ? std::optional<T> (variant.value<T>())
          : std::optional<T> (std::nullopt);
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
