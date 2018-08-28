#include <logging/message.hpp>

#include <util-generic/hostname.hpp>
#include <util-generic/syscall.hpp>

#include <stdexcept>

namespace fhg
{
  namespace logging
  {
    message::message ( decltype (_content) content
                     , decltype (_category) category
                     )
      : message ( std::move (content)
                , std::move (category)
                , decltype (_timestamp)::clock::now()
                , util::hostname()
                , util::syscall::getpid()
                , util::syscall::gettid()
                )
    {}
    message::message ( decltype (_content) content
                     , decltype (_category) category
                     , decltype (_timestamp) timestamp
                     , decltype (_hostname) hostname
                     , decltype (_process_id) process_id
                     , decltype (_thread_id) thread_id
                     )
      : _content (std::move (content))
      , _category (std::move (category))
      , _timestamp (std::move (timestamp))
      , _hostname (std::move (hostname))
      , _process_id (std::move (process_id))
      , _thread_id (std::move (thread_id))
    {}

    namespace
    {
      template<typename Timepoint, typename Rep>
        Timepoint to_std_timepoint (Rep seconds_since_epoch)
      {
        using namespace std::chrono;
        return Timepoint
          ( duration_cast<typename Timepoint::duration>
              (duration<Rep, seconds::period> (seconds_since_epoch))
          );
      }
    }

    message message::from_legacy (legacy::event const& event)
    {
      auto const category
        ( event.severity() == legacy::TRACE ? legacy::category_level_trace
        : event.severity() == legacy::INFO ? legacy::category_level_info
        : event.severity() == legacy::WARN ? legacy::category_level_warn
        : event.severity() == legacy::ERROR ? legacy::category_level_error
        : throw std::logic_error ("bad legacy severity")
        );

      return message
             ( event.message()
             , category
             , to_std_timepoint<decltype (_timestamp)> (event.tstamp())
             , event.host()
             , event.pid()
             , event.tid()
             );
    }
  }
}
