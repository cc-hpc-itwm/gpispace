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

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      template<typename T>
        inline T* first_parent_being_a (QWidget* widget)
      {
        while (widget && !qobject_cast<T*> (widget))
        {
          widget = widget->parentWidget();
        }
        return qobject_cast<T*> (widget);
      }

      template<typename T>
        inline T* first_parent_being_a (QObject* object)
      {
        while (object && !qobject_cast<T*> (object))
        {
          object = object->parent();
        }
        return qobject_cast<T*> (object);
      }
    }
  }
}
