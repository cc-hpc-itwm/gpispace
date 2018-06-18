#include <logging/legacy/receiver.hpp>

#include <fhglog/appender/call.hpp>

namespace fhg
{
  namespace logging
  {
    namespace legacy
    {
      receiver::receiver (unsigned short const port)
        : _io_service (1)
        , _logger()
        , _server (_logger, _io_service, port)
        , _thread ([this] { _io_service.run(); })
      {
        _logger.setLevel (TRACE);
        _logger.addAppender<appender::call>
          ([this] (event const& event) { on_legacy (event); });
      }

      receiver::~receiver()
      {
        //! \note Legacy LogServer does not properly terminate
        //! receiving, so has to be forced to stop from outside. This
        //! is also why the thread is started after creating the
        //! server: on shutdown, the thread needs to be joined before
        //! the server is destructed since the jobs queued for the
        //! thread access the server.
        _io_service.stop();
      }
    }
  }
}
