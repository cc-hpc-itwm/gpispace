#include <fhglog/appender/swftrace.hpp>

namespace fhg
{
  namespace log
  {
    SWFTraceAppender::SWFTraceAppender (boost::filesystem::path const& path)
      : _path (path)
    {
      _stream.exceptions (std::ios_base::badbit | std::ios_base::failbit);
    }

    void SWFTraceAppender::open_file()
    {
      if (!_stream.is_open())
      {
        _stream.open (_path.string(), std::ios_base::out | std::ios_base::app);
      }
    }

    void SWFTraceAppender::append_header (std::string const& header)
    {
      open_file();
      _stream << header << std::flush;
    }

    void SWFTraceAppender::append (SWFTraceEvent const& event)
    {
      open_file();
      _stream << event << std::endl;
    }
  }
}
