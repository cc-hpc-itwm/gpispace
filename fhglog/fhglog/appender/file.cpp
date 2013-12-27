// alexander.petry@itwm.fraunhofer.de

#include "file.hpp"

namespace fhg
{
  namespace log
  {
    FileAppender::FileAppender ( const std::string& path
                               , const std::string& fmt
                               , int flush_interval
                               , const std::ios_base::openmode& mode
                               )
      : _stream (path.c_str(), mode)
      , _format (fmt)
      , _flush_interval (flush_interval)
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
      _stream << format (_format, event);

      if (++_event_count >= _flush_interval)
      {
        flush();
        _event_count = 0;
      }
    }
  }
}
