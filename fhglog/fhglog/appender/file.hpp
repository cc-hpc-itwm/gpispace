// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_LOG_FILE_APPENDER_HPP
#define FHG_LOG_FILE_APPENDER_HPP 1

#include <string>
#include <fstream>
#include <stdexcept>

#include <fhglog/format.hpp>
#include <fhglog/Appender.hpp>

#include <boost/shared_ptr.hpp>

namespace fhg
{
 namespace log
 {
   class FileAppender : public Appender
   {
   public:
     typedef boost::shared_ptr<FileAppender> ptr_t;

     FileAppender ( const std::string& path
                  , const std::string& format
                  , int flush_interval = 5
                  , const std::ios_base::openmode& mode
                  = std::ios_base::out
                  | std::ios_base::app
                  | std::ios_base::binary
                  );

     virtual void flush();
     virtual void append (const LogEvent&);

   private:
     std::ofstream _stream;
     std::string const _format;
     int const _flush_interval;
     int _event_count;
   };
 }
}

#endif
