#include <time.h>

#include <map>
#include <string>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/stats.hpp>

class StatisticsImpl : FHG_PLUGIN
                     , public stats::Statistics
{
public:
  StatisticsImpl () {}
  ~StatisticsImpl (){}

  FHG_PLUGIN_START(kernel)
  {
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP(kernel)
  {
    std::cout << "*** Statistics gathered ***" << std::endl;
    std::cout << std::endl;
    std::cout << "  [timer]" << std::endl;
    for ( std::map<std::string,time_t>::const_iterator it (m_timer.begin())
        ; it != m_timer.end()
        ; ++it
        )
    {
      std::cout << "  " << it->first << " = " << it->second << std::endl;
    }

    std::cout << std::endl;
    std::cout << "  [counter]" << std::endl;
    for ( std::map<std::string,size_t>::const_iterator it (m_counter.begin())
        ; it != m_counter.end()
        ; ++it
        )
    {
      std::cout << "  " << it->first << " = " << it->second << std::endl;
    }
    FHG_PLUGIN_STOPPED();
  }

  void inc(const char * counter)
  {
    ++m_counter[counter];
  }

  void dec(const char * counter)
  {
    --m_counter[counter];
  }

  void start_timer (const char * timer)
  {
    m_timer[timer] -= time(NULL);
  }

  void stop_timer (const char * timer)
  {
    m_timer[timer] += time(NULL);
  }

  size_t get_counter(const char *name) const
  {
    if (m_counter.find(name) == m_counter.end())
      return 0;
    else
      return m_counter.find(name)->second;
  }

  size_t get_time(const char *name) const
  {
    if (m_timer.find(name) == m_timer.end())
      return 0;
    else
      return m_timer.find(name)->second;
  }
private:
  std::map<std::string, size_t> m_counter;
  std::map<std::string, time_t> m_timer;
};

EXPORT_FHG_PLUGIN( simple_stats
                 , StatisticsImpl
                 , "a simple statistics plugin"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , ""
                 , ""
                 );
