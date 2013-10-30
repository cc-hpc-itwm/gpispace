#include <sys/time.h>

#include <map>
#include <string>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/stats.hpp>

namespace detail
{
  static double now()
  {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + (tv.tv_usec / (1000.0 * 1000.0));
  }
}

class StatisticsImpl : FHG_PLUGIN
                     , public stats::Statistics
{
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;
  typedef boost::condition_variable_any condition_type;
public:
  FHG_PLUGIN_START()
  {
    fhg_kernel()->storage()->load("counter", m_counter);
    fhg_kernel()->storage()->load("timer", m_timer);
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    fhg_kernel()->storage()->save("counter", m_counter);
    fhg_kernel()->storage()->save("timer", m_timer);
    print(std::cout);
    FHG_PLUGIN_STOPPED();
  }

  void fhg_on_plugin_loaded(std::string const &)
  {
    inc("plugin.loaded");
  }

  void fhg_on_plugin_unload(std::string const &)
  {
    inc("plugin.unload");
  }

  void inc(const char * counter)
  {
    lock_type l(mtx_count);
    ++m_counter[counter];
  }

  void dec(const char * counter)
  {
    lock_type l(mtx_count);
    --m_counter[counter];
  }

  void start_timer (const char * timer)
  {
    lock_type l(mtx_timer);
    m_timer[timer] -= detail::now();
  }

  void stop_timer (const char * timer)
  {
    lock_type l(mtx_timer);
    m_timer[timer] += detail::now();
  }

  size_t count(const char *name) const
  {
    lock_type l(mtx_count);
    if (m_counter.find(name) == m_counter.end())
      return 0;
    else
      return m_counter.find(name)->second;
  }

  double timer(const char *name) const
  {
    lock_type l(mtx_timer);
    if (m_timer.find(name) == m_timer.end())
      return 0;
    else
      return m_timer.find(name)->second;
  }

  void print(std::ostream &os) const
  {
    os << "*** Statistics gathered ***" << std::endl;
    os << std::endl;
    os << "  [timer]" << std::endl;
    {
      lock_type l(mtx_timer);
      for ( std::map<std::string,double>::const_iterator it (m_timer.begin())
          ; it != m_timer.end()
          ; ++it
          )
      {
        os << "  " << it->first << " = " << it->second << std::endl;
      }
    }

    os << std::endl;
    os << "  [counter]" << std::endl;
    {
      lock_type l(mtx_count);
      for ( std::map<std::string,size_t>::const_iterator it (m_counter.begin())
          ; it != m_counter.end()
          ; ++it
          )
      {
        os << "  " << it->first << " = " << it->second << std::endl;
      }
    }
  }
private:
  mutable mutex_type mtx_timer;
  mutable mutex_type mtx_count;

  std::map<std::string, size_t> m_counter;
  std::map<std::string, double> m_timer;
};

EXPORT_FHG_PLUGIN( stats
                 , StatisticsImpl
                 , "stats"
                 , "a simple statistics plugin"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , ""
                 , ""
                 );
