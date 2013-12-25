// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_LOG_FILTERING_APPENDER_HPP
#define FHG_LOG_FILTERING_APPENDER_HPP 1

#include <fhglog/Appender.hpp>
#include <fhglog/Filter.hpp>

namespace fhg
{
  namespace log
  {
    class FilteringAppender : public Appender
    {
    public:

      FilteringAppender ( const Appender::ptr_t& appender
                        , const Filter::ptr_t& filter
                        )
        : _appender (appender)
        , _filter (filter)
      {}

      virtual void append (const LogEvent &evt)
      {
        if (! (*_filter)(evt))
        {
          _appender->append (evt);
        }
      }
      virtual void flush()
      {
        _appender->flush();
      }

    private:

      Appender::ptr_t _appender;
      Filter::ptr_t _filter;
    };
  }
}

#endif
