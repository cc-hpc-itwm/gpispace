// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/stat.hpp>
#include <fhg/util/now.hpp>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/range/adaptor/map.hpp>

#include <iomanip>
#include <ostream>
#include <set>

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
      }
      void reset ()
      {
        count_map ().clear ();
        time_map ().clear ();
      }
      void inc (const std::string& k)
      {
        ++count_map()[k];
      }
      void start (const std::string& k)
      {
        time_map()[k] -= now ();
      }
      void stop (const std::string& k)
      {
        time_map()[k] += now ();
      }
      void out (std::ostream& s)
      {
        std::set<std::string> keys;

        BOOST_FOREACH ( const std::string& key
                      , count_map() | boost::adaptors::map_keys
                      )
        {
          keys.insert (key);
        }
        BOOST_FOREACH ( const std::string& key
                      , time_map() | boost::adaptors::map_keys
                      )
        {
          keys.insert (key);
        }

        BOOST_FOREACH (const std::string& key, keys)
        {
          s << "STAT"
            << " " << std::setw(12) << count_map()[key]
            << " " << std::setw(12) << time_map()[key]
            << " " << key
            << std::endl
            ;
        }
      }
    }
  }
}
