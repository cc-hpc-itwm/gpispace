// alexander.petry@itwm.fraunhofer.de

#pragma once

#include <fhglog/Appender.hpp>

#include <iostream>

namespace fhg
{
  namespace log
  {
    class StreamAppender : public Appender
    {
    public:
      StreamAppender ( std::ostream&
                     , std::string const& format
                     );

      virtual void append (const LogEvent&) override;
      virtual void flush() override;

    private:
      std::ostream& _stream;
      std::string const _format;
    };
  }
}
