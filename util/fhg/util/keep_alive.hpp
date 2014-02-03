// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_KEEP_ALIVE_HPP
#define FHG_UTIL_KEEP_ALIVE_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

namespace fhg
{
  namespace util
  {
    class keep_alive
    {
    public:
      keep_alive ( boost::function<bool()> check
                 , boost::function<void()> on_fail
                 , std::size_t maximum_failures
                 , boost::posix_time::time_duration interval
                 )
        : _maximum_failures (maximum_failures)
        , _failures()
        , _interval (interval)
        , _check (check)
        , _on_fail (on_fail)
        , _check_thread (&keep_alive::check, this)
      {
      }

      ~keep_alive()
      {
        _check_thread.interrupt();
        if (_check_thread.joinable())
        {
          _check_thread.join();
        }
      }

      void check()
      {
        while (true)
        {
          if (!_check())
          {
            ++_failures;

            if (_maximum_failures >= _failures)
            {
              _on_fail();
              return;
            }
          }
          else
          {
            _failures = 0;
          }

          boost::this_thread::sleep (_interval);
        }
      }

      std::size_t _maximum_failures;
      std::size_t _failures;

      boost::posix_time::time_duration _interval;

      boost::function<bool()> _check;
      boost::function<void()> _on_fail;

      boost::thread _check_thread;
    };
  }
}

#endif
