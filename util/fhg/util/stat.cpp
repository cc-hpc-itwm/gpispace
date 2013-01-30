// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/stat.hpp>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include <iomanip>
#include <ostream>

#include <sys/time.h>

namespace fhg
{
  namespace util
  {
    namespace stat
    {
      namespace
      {
        typedef boost::unordered_map<std::string, long> count_map_type;
        typedef boost::unordered_map<std::string, double> time_map_type;

        count_map_type& count_map()
        {
          static count_map_type m;

          return m;
        }
        time_map_type& time_map()
        {
          static time_map_type m;

          return m;
        }

        double current_time()
        {
          struct timeval tv;

          gettimeofday (&tv, NULL);

          return (double(tv.tv_sec) + double (tv.tv_usec) * 1E-6);
        }
      }
      void inc (const std::string& k)
      {
        ++count_map()[k];
      }
      void start (const std::string& k)
      {
        time_map()[k] -= current_time();
      }
      void stop (const std::string& k)
      {
        time_map()[k] += current_time();
      }
      void out (std::ostream& s)
      {
        typedef std::pair<std::string, long> sl_type;

        BOOST_FOREACH (const sl_type& sl, count_map())
        {
          s << "STAT"
            << " " << std::setw(12) << sl.second
            << " " << std::setw(12) << time_map()[sl.first]
            << " " << sl.first
            << std::endl
            ;
        }
      }
    }
  }
}
