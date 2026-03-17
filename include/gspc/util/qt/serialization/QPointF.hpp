#pragma once

#include <QtCore/QPointF>

namespace boost
{
  namespace serialization
  {
    template<class Archive>
      inline void serialize (Archive& ar, QPointF& point, unsigned int const)
    {
      ar & point.rx();
      ar & point.ry();
    }
  }
}
