// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_LOG_APPENDER_CALL_HPP
#define FHG_LOG_APPENDER_CALL_HPP

#include <fhglog/Appender.hpp>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace fhg
{
 namespace log
 {
   namespace appender
   {
     class call : public Appender
     {
     public:
       typedef boost::function<void (const LogEvent&)> handler_t;

       call (const handler_t& handler)
       : _handler (handler)
       {}

       void append (const LogEvent& evt)
       {
         _handler (evt);
       }

       void flush()
       {}

     private:
       handler_t _handler;
     };
   }
 }
}

#endif
