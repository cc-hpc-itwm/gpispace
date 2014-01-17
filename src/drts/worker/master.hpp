#ifndef DRTS_MASTER_HPP
#define DRTS_MASTER_HPP 1

#include <string>
#include <list>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/utility.hpp>

namespace drts
{
  class Master : boost::noncopyable
  {
    typedef boost::mutex mutex_type;
    typedef boost::condition_variable condition_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef boost::posix_time::ptime time_type;
  public:

    explicit Master(std::string const& name);

    inline bool is_connected () const
    {
      return _is_connected;
    }

    inline void is_connected (bool b)
    {
      _is_connected = b;
    }

    std::string const & name() const { return m_name; }

  private:
    mutable mutex_type m_stats_mutex;

    std::string m_name;
    bool _is_connected;
  };
}

#endif // DRTS_MASTER_HPP
