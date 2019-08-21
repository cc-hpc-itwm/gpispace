// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <fhglog/Appender.hpp>

#include <functional>

namespace fhg
{
 namespace log
 {
   namespace appender
   {
     class call : public Appender
     {
     public:
       typedef std::function<void (const LogEvent&)> handler_t;

       call (const handler_t& handler)
       : _handler (handler)
       {}

       virtual void append (const LogEvent& evt) override
       {
         _handler (evt);
       }

       virtual void flush() override
       {}

     private:
       handler_t _handler;
     };
   }
 }
}
