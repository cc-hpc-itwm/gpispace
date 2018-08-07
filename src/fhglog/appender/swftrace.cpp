
#include <fhglog/appender/swftrace.hpp>

namespace fhg
{
  namespace log
  {
    SWFTraceAppender::SWFTraceAppender(boost::filesystem::path const& path)
      : _path (path.string())
      , _event_count (0)
    {
      _stream.exceptions (std::ios_base::badbit | std::ios_base::failbit);
    }

    SWFTraceAppender::SWFTraceAppender(SWFTraceAppender const& app)
      : _path (app.path())
      , _event_count (0)
    {
        _stream.exceptions (std::ios_base::badbit | std::ios_base::failbit);
    }


    std::string SWFTraceAppender::path() const
    {
      return _path;
    }

    void SWFTraceAppender::flush()
    {
      _stream.flush();
    }

    void SWFTraceAppender::open_file()
    {
      if (!_stream.is_open())
      {
        _stream.open    // write in text mode
          (_path
          , std::ios_base::out | std::ios_base::app
          );
      }
    }


    void SWFTraceAppender::append_header(std::string const& header)
    {
      open_file();
      _stream << header;
      flush();
    }

    void SWFTraceAppender::append(SWFTraceEvent const& event)
    {
      open_file();
      _stream << event.string();
      if (++_event_count >= 5)
      {
        flush();
        _event_count = 0;
      }
    }
  }
}
