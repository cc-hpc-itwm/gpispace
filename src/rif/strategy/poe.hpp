// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_RIF_STRATEGY_POE_HPP
#define FHG_RIF_STRATEGY_POE_HPP

#include <rif/entry_point.hpp>

#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      namespace poe
      {
        void bootstrap ( std::vector<std::string> const& hostnames
                       , boost::optional<unsigned short> const& port
                       , std::string const& register_host
                       , unsigned short register_port
                       , boost::filesystem::path const& binary
                       );
        void teardown ( std::vector<fhg::rif::entry_point> const& entry_points
                      , std::vector<fhg::rif::entry_point>& failed_entry_points
                      );
      }
    }
  }
}

#endif
