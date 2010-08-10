#ifndef FHG_LOG_LOGGER_CONFIG_HPP
#define FHG_LOG_LOGGER_CONFIG_HPP 1

#include <boost/function.hpp>

#include <fhglog/memory.hpp>
#include <fhglog/types.hpp>

namespace fhg
{
  namespace log
  {
    namespace detail
    {
      struct dev_null
      {
        void operator() (const fhg::log::LogEvent &) const
        {
          return;
        }
      };
    }

    class logger_config
    {
    public:
      typedef shared_ptr<logger_config> ptr_t;

      logger_config ()
        : level_()
        , sink_( detail::dev_null () )
      {}

      template <typename Level>
      void level(Level const & lvl)
      {
        level_ = lvl;
      }
      LogLevel const & level(void) const
      {
        return level_;
      }

      template <typename L>
      bool enabled(L const & l) const
      {
        return l >= level_;
      }

      template <typename S>
      void sink(S s)
      {
        sink_ = s;
      }

      sink_t const & sink(void) const
      {
        return sink_;
      }
    private:
      LogLevel level_;
      sink_t sink_;
    };
  }
}

#endif
