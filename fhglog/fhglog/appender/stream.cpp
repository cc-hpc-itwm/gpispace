// alexander.petry@itwm.fraunhofer.de

#include "stream.hpp"

#include <fhglog/format.hpp>

namespace fhg
{
  namespace log
  {
    StreamAppender::StreamAppender ( std::ostream& stream
                                   , std::string const& format
                                   , StreamAppender::ColorMode color_mode
                                   )
      : _stream (stream)
      , _format (format)
      , _color_mode (color_mode)
    {}

    void StreamAppender::append (const LogEvent& event)
    {
      if (_color_mode == COLOR_ON)
      {
        _stream << _color_map[event.severity()];
      }

      // TODO: pass color mapper into the formatter
      _stream << format (_format, event);

      if (_color_mode == COLOR_ON)
      {
        _stream << _color_map.reset_escape_code();
      }
    }

    void StreamAppender::flush()
    {
      _stream.flush();
    }
  }
}

