// bernd.loerwald@itwm.fraunhofer.de

#pragma once

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
      namespace ssh
      {
        void bootstrap ( std::vector<std::string> const& hostnames
                       , boost::optional<unsigned short> const& port
                       , std::string const& register_host
                       , unsigned short register_port
                       , boost::filesystem::path const& binary
                       , std::vector<std::string> const& parameters
                       );
        void teardown ( std::vector<fhg::rif::entry_point> const& entry_points
                      , std::vector<fhg::rif::entry_point>& failed_entry_points
                      , std::vector<std::string> const& parameters
                      );
      }
    }
  }
}
