// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <QObject>
#include <QVariant>
#include <QString>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class scoped_property_setter
      {
      private:
        QObject* _object;
        const QByteArray _name;
        const QVariant _old_value;

      public:
        scoped_property_setter ( QObject* object
                               , const QString& name
                               , const QVariant& new_value
                               )
          : _object (object)
          , _name (name.toAscii())
          , _old_value (_object->property (_name.data()))
        {
          _object->setProperty (_name.data(), new_value);
        }
        ~scoped_property_setter()
        {
          _object->setProperty (_name.data(), _old_value);
        }
      };
    }
  }
}
