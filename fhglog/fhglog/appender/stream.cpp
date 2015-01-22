// alexander.petry@itwm.fraunhofer.de

#include <fhglog/appender/stream.hpp>

#include <fhglog/format.hpp>

namespace fhg
{
  namespace log
  {
    StreamAppender::StreamAppender ( std::ostream& stream
                                   , std::string const& format
                                   )
      : _stream (stream)
      , _format (format)
    {}

    void StreamAppender::append (const LogEvent& event)
    {
      format (_stream, _format, event);
    }

    void StreamAppender::flush()
    {
      _stream.flush();
    }
  }
}
