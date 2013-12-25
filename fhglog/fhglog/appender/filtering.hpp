/*
 * =====================================================================================
 *
 *       Filename:  FilteringAppender.hpp
 *
 *    Description:  an appender that filters events before appending
 *
 *        Version:  1.0
 *        Created:  10/12/2009 10:57:28 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHG_LOG_FILTERING_APPENDER_HPP
#define FHG_LOG_FILTERING_APPENDER_HPP 1

#include <fhglog/Appender.hpp>
#include <fhglog/Filter.hpp>

namespace fhg { namespace log {
  class FilteringAppender : public Appender
  {
    public:
      FilteringAppender(const Appender::ptr_t &real_appender, const Filter::ptr_t &a_filter)
        : _appender (real_appender)
        , filter_(a_filter) { }

      virtual void append(const LogEvent &evt)
      {
        if (! (*filter_)(evt))
        {
          _appender->append(evt);
        }
      }
    virtual void flush()
    {
      _appender->flush();
    }
    private:
    Appender::ptr_t _appender;
      Filter::ptr_t filter_;
  };
}}

#endif
