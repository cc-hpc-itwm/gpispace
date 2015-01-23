// alexander.petry@itwm.fraunhofer.de

#include <fhglog/appender/file.hpp>

#include <fhglog/format.hpp>

namespace fhg
{
  namespace log
  {
    FileAppender::FileAppender ( const std::string& path
                               , const std::string& fmt
                               )
      : _path (path)
      , _format (fmt)
      , _event_count (0)
    {
      _stream.exceptions (std::ios_base::badbit | std::ios_base::failbit);
    }

    void FileAppender::flush()
    {
      _stream.flush();
    }

    void FileAppender::append (const LogEvent& event)
    {
      if (!_stream.is_open())
      {
        _stream.open
          (_path
          , std::ios_base::out | std::ios_base::app | std::ios_base::binary
          );
      }

      format (_stream, _format, event);

      if (++_event_count >= 5)
      {
        flush();
        _event_count = 0;
      }
    }
  }
}
