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

#include <util/qt/scoped_disconnect.hpp>

#include <QObject>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      scoped_disconnect::scoped_disconnect ( const QObject* const sender
                                           , const char* const signal
                                           , const QObject* const receiver
                                           , const char* const method
                                           )
        : _sender (sender)
        , _signal (signal)
        , _receiver (receiver)
        , _method (method)
        , _was_disconnected
            (QObject::disconnect (_sender, _signal, _receiver, _method))
      {
      }

      scoped_disconnect::~scoped_disconnect()
      {
        if (_was_disconnected)
        {
          QObject::connect (_sender, _signal, _receiver, _method);
        }
      }
    }
  }
}
