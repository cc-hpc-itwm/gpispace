/*
 * =====================================================================================
 *
 *       Filename:  NullAppender.hpp
 *
 *    Description:  a "Null" appender, an appender that just does nothing
 *
 *        Version:  1.0
 *        Created:  09/21/2009 12:04:03 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */


#ifndef  FHG_LOG_NULLAPPENDER_HPP_INC
#define  FHG_LOG_NULLAPPENDER_HPP_INC

#include <fhglog/Appender.hpp>

namespace fhg { namespace log {
  class NullAppender : public Appender
  {
  public:
    explicit
    NullAppender(const std::string &a_name = "null")
      : Appender(a_name) { }

    virtual void append(const LogEvent &) const { }
  };
}}
#endif   /* ----- #ifndef FHG_LOG_NULLAPPENDER_HPP_INC  ----- */
