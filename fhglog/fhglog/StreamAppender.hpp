/*
 * =====================================================================================
 *
 *       Filename:  StreamAppender.hpp
 *
 *    Description:  A very simple appender that logs to a user-defined stream
 *
 *        Version:  1.0
 *        Created:  08/25/2009 10:47:11 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */


#ifndef  STREAM_APPENDER_INC
#define  STREAM_APPENDER_INC

#include <iostream>
#include <fhglog/Appender.hpp>
#include <fhglog/color_map.hpp>

namespace fhg { namespace log {
    class StreamAppender : public Appender
    {
    public:
      enum ColorMode
        {
          COLOR_OFF
          , COLOR_ON
          , COLOR_AUTO
        };

      /*
       * Create a new StreamAppender with the given name.
       *
       * The appender does not take ownership of the stream, i.e. you have to
       * close the stream on your own.
       */
      StreamAppender( const std::string &a_name
                    , std::ostream &stream
                    , std::string const & fmt
                    , ColorMode color_mode = COLOR_AUTO
                    );

      void append(const LogEvent &evt);
      void flush (void);
    private:
      std::ostream &stream_;
      std::string fmt_;
      ColorMode color_mode_;
      color_map_t color_map;
    };
}}
#endif   /* ----- #ifndef STREAM_APPENDER_INC  ----- */
