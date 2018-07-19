
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
   class SWFTraceAppender //: public Appender
   {
   public:
     SWFTraceAppender(boost::filesystem::path const& path);
     SWFTraceAppender(SWFTraceAppender const& app);

     std::string path() const;
     void flush();
     void append(SWFTraceEvent const& event);
     void append_header(std::string const& header);

   private:
     std::string const _path;
     std::ofstream _stream;
     int _event_count;

     void open_file();
   };
 }
}
