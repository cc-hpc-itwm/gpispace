
#pragma once

#include <fhglog/Appender.hpp>
#include <fhglog/swftrace_event.hpp>

#include <boost/filesystem.hpp>

#include <string>
#include <fstream>

namespace fhg
{
 namespace log
 {
   class SWFTraceAppender
   {
   public:
     SWFTraceAppender(boost::filesystem::path const&);
     SWFTraceAppender(SWFTraceAppender const&);

     std::string path() const;
     void flush();
     void append(SWFTraceEvent const&);
     void append_header(std::string const&);

   private:
     std::string const _path;
     std::ofstream _stream;
     int _event_count;

     void open_file();
   };
 }
}
