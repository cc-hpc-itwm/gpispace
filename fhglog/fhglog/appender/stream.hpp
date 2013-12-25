// alexander.petry@itwm.fraunhofer.de

#ifndef STREAM_APPENDER_INC
#define STREAM_APPENDER_INC

#include <fhglog/Appender.hpp>
#include <fhglog/color_map.hpp>

#include <iostream>

namespace fhg
{
  namespace log
  {
    class StreamAppender : public Appender
    {
    public:
      enum ColorMode
        { COLOR_OFF
        , COLOR_ON
        };

      StreamAppender ( std::ostream&
                     , std::string const& format
                     , ColorMode color_mode = COLOR_OFF
                     );

      virtual void append (const LogEvent&);
      virtual void flush();

    private:
      std::ostream& _stream;
      std::string const _format;
      ColorMode const _color_mode;
      color_map_t const _color_map;
    };
  }
}

#endif
