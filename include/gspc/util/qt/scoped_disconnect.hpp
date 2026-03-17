// Copyright (C) 2013,2015,2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

class QObject;



    namespace gspc::util::qt
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
