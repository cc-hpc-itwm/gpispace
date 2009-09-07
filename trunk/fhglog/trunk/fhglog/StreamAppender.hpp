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
      StreamAppender(const std::string &name = "console", std::ostream &stream = std::clog)
        : name_(name), fmt_(Formatter::DefaultFormatter()), stream_(stream) {}
      ~StreamAppender() {}

      void setFormat(Formatter::ptr_t fmt) { fmt_ = fmt; }
      void setFormat(Formatter *fmt) { fmt_ = Formatter::ptr_t(fmt); }
      void append(const LogEvent &evt);
      const std::string &name() const { return name_; }
    private:
      std::string name_;
      Formatter::ptr_t fmt_;
      std::ostream &stream_;
  };
}}
#endif   /* ----- #ifndef STREAM_APPENDER_INC  ----- */
