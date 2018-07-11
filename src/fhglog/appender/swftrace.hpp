
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
     SWFTraceAppender(const boost::filesystem::path path);
     SWFTraceAppender(const SWFTraceAppender& app);

     std::string path() const;
     void flush();
     void append(const SWFTraceEvent& event);
     void append_header(const std::string& header);

   private:
     std::string const _path;
     std::ofstream _stream;
     int _event_count;

     void open_file();
   };
 }
}
