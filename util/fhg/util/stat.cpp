// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/stat.hpp>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include <iomanip>
#include <ostream>

namespace fhg
{
  namespace util
  {
    namespace stat
    {
      namespace
      {
        typedef boost::unordered_map<std::string, long> cnt_map_type;

        boost::unordered_map<std::string, cnt_map_type>& stat_map()
        {
          static boost::unordered_map<std::string, cnt_map_type> m;

          return m;
        }
      }
      void inc (const std::string& k, const std::string& s)
      {
        ++stat_map()[k][s];
      }
      void out (std::ostream& s)
      {
        typedef std::pair<std::string, cnt_map_type> scm_type;

        BOOST_FOREACH (const scm_type& scm, stat_map())
        {
          s << scm.first << std::endl;

          typedef std::pair<std::string, long> sl_type;

          BOOST_FOREACH (const sl_type& sl, scm.second)
          {
            s << "  " << std::setw(12) << sl.second
              << " " << sl.first << std::endl;
          }
        }
      }
    }
  }
}
