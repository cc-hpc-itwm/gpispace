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

#include	<iostream>
#include    <fhglog/Appender.hpp>

namespace fhg { namespace log {
  class StreamAppender : public Appender
  {
    public:
      explicit
      /* 
      * Create a new StreamAppender with the given name.
      *
      * The appender does not take ownership of the stream, i.e. you have to
      * close the stream on your own.
      */
      StreamAppender(const std::string &a_name = "console", std::ostream &stream = std::clog)
        : Appender(a_name), stream_(stream) {}
      ~StreamAppender() {}

      void append(const LogEvent &evt) const;
    private:
      std::ostream &stream_;
  };
}}
#endif   /* ----- #ifndef STREAM_APPENDER_INC  ----- */
