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
          ( const QObject* const sender, const char* const signal
          , const QObject* const receiver, const char* const method
          );
        ~scoped_disconnect();

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
