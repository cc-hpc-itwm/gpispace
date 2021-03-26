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

#include <QWidget>
#include <QObject>
#include <QGraphicsItem>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      template<typename T> T throwing_qobject_cast (QObject* from)
      {
        T x (qobject_cast<T> (from));

        if (!x)
        {
          throw std::runtime_error ("throwing_qobject_cast failed");
        }

        return x;
      }
      template<typename T> T throwing_qobject_cast (const QObject* from)
      {
        T x (qobject_cast<T> (from));

        if (!x)
        {
          throw std::runtime_error ("throwing_qobject_cast failed");
        }

        return x;
      }
      template<typename T> T throwing_qgraphicsitem_cast (QGraphicsItem* from)
      {
        T x (qgraphicsitem_cast<T> (from));

        if (!x)
        {
          throw std::runtime_error ("throwing_qgraphicsitem_cast failed");
        }

        return x;
      }
    }
  }
}
