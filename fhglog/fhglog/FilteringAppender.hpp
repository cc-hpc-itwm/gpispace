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

#include <fhglog/Filter.hpp>
#include <fhglog/DecoratingAppender.hpp>

namespace fhg { namespace log {
  class FilteringAppender : public DecoratingAppender
  {
    public:
      FilteringAppender(const Appender::ptr_t &real_appender, const Filter::ptr_t &a_filter)
        : DecoratingAppender(real_appender, "-filtered")
        , filter_(a_filter) { }

      FilteringAppender(Appender *real_appender, const Filter::ptr_t &a_filter)
        : DecoratingAppender(real_appender, "-filtered")
        , filter_(a_filter) { }

      virtual void append(const LogEvent &evt)
      {
        if (! (*filter_)(evt))
        {
          DecoratingAppender::append(evt);
        }
      }
    private:
      Filter::ptr_t filter_;
  };
}}

#endif
