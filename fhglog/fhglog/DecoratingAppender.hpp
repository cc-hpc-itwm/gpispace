/*
 * =====================================================================================
 *
 *       Filename:  DecoratingAppender.hpp
 *
 *    Description:  decorates an appender
 *
 *        Version:  1.0
 *        Created:  10/13/2009 12:44:05 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHG_LOG_DECORATING_APPENDER_HPP
#define FHG_LOG_DECORATING_APPENDER_HPP 1

#include <fhglog/Appender.hpp>

namespace fhg { namespace log {
  class DecoratingAppender : public Appender
  {
    public:
      explicit
      DecoratingAppender(const Appender::ptr_t &real_appender, const std::string &name_tag = "")
        : Appender(real_appender->name() + name_tag)
        , real_appender_(real_appender)
      {
      }

      explicit
      DecoratingAppender(Appender *real_appender, const std::string &name_tag = "")
        : Appender(real_appender->name() + name_tag)
        , real_appender_(real_appender)
      {
      }

      virtual void append(const LogEvent &evt)
      {
        real_appender_->append(evt);
      }

      const Appender::ptr_t &decorated() const
      {
        return real_appender_;
      }

    void flush(void)
    {
      real_appender_->flush();
    }
    private:
      Appender::ptr_t real_appender_;
  };
}}

#endif
