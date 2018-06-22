#include <logging/legacy/emitter.hpp>

#include <logging/legacy/event.hpp>

namespace fhg
{
  namespace logging
  {
    namespace legacy
    {
      emitter::emitter (std::string const& hostname, unsigned short port)
        : _appender (hostname, std::to_string (port), _io_service)
      {}

      void emitter::trace (std::string const& message)
      {
        _appender.append ({TRACE, message});
      }
      void emitter::info (std::string const& message)
      {
        _appender.append ({INFO, message});
      }
      void emitter::warn (std::string const& message)
      {
        _appender.append ({WARN, message});
      }
      void emitter::error (std::string const& message)
      {
        _appender.append ({ERROR, message});
      }
    }
  }
}
