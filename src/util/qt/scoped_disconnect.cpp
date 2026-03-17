// Copyright (C) 2013,2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/qt/scoped_disconnect.hpp>

#include <QObject>



    namespace gspc::util::qt
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
