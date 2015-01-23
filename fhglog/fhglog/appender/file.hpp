// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_LOG_FILE_APPENDER_HPP
#define FHG_LOG_FILE_APPENDER_HPP 1

#include <string>
#include <fstream>

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

#endif
