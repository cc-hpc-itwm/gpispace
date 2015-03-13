// alexander.petry@itwm.fraunhofer.de

#pragma once

#include <string>
#include <fstream>

#include <fhglog/Appender.hpp>

namespace fhg
{
 namespace log
 {
   class FileAppender : public Appender
   {
   public:
     FileAppender ( const std::string& path
                  , const std::string& format
                  );

     virtual void flush() override;
     virtual void append (const LogEvent&) override;

   private:
     std::string const _path;
     std::ofstream _stream;
     std::string const _format;
     int _event_count;
   };
 }
}
