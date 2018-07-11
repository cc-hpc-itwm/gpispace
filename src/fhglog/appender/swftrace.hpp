#pragma once

#include <fhglog/swftrace_event.hpp>

#include <boost/filesystem/path.hpp>

#include <string>
#include <fstream>

namespace fhg
{
  namespace log
  {
    class SWFTraceAppender
    {
    public:
      SWFTraceAppender (boost::filesystem::path const&);

      void append (SWFTraceEvent const&);
      void append_header (std::string const&);

    private:
      boost::filesystem::path const _path;
      std::ofstream _stream;

      void open_file();
    };
  }
}
