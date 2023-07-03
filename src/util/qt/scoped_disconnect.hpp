// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

class QObject;

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      struct scoped_disconnect
      {
        scoped_disconnect
          ( const QObject* sender, const char* signal
          , const QObject* receiver, const char* method
          );
        ~scoped_disconnect();
        scoped_disconnect (scoped_disconnect const&) = delete;
        scoped_disconnect (scoped_disconnect&&) = delete;
        scoped_disconnect& operator= (scoped_disconnect const&) = delete;
        scoped_disconnect& operator= (scoped_disconnect&&) = delete;

      private:
        const QObject* const _sender;
        const char* const _signal;
        const QObject* const _receiver;
        const char* const _method;

        const bool _was_disconnected;
      };
    }
  }
}
