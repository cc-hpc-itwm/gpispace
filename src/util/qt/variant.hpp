// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhg/util/backtracing_exception.hpp>

#include <boost/optional.hpp>

#include <QVariant>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      template<typename T> bool stores (const QVariant variant)
      {
        return qMetaTypeId<T>() == variant.userType();
      }

      template<typename T> T value (const QVariant variant)
      {
        if (stores<T> (variant))
        {
          return variant.value<T>();
        }

        throw fhg::util::backtracing_exception
          ("value<T> (variant): QVariant contained wrong type.");
      }

      template<typename T> boost::optional<T> optional (QVariant variant)
      {
        return stores<T> (variant)
          ? boost::optional<T> (value<T> (variant))
          : boost::optional<T> (boost::none);
      }

      template<typename T> QList<T> collect (const QVariant variant)
      {
        QList<T> result;

        if (stores<T> (variant))
        {
          result << value<T> (variant);
        }
        else if (stores<QVariantList> (variant))
        {
          for (const QVariant v : value <QVariantList> (variant))
          {
            result << collect<T> (v);
          }
        }

        return result;
      }
    }
  }
}
