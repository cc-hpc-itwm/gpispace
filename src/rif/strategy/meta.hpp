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
      std::vector<std::string> available_strategies();

      std::vector<fhg::rif::entry_point> bootstrap
        ( std::string const& strategy
        , std::vector<std::string> const& hostnames
        , boost::optional<unsigned short> const& port
        , boost::filesystem::path const& gspc_home
        );
      void teardown ( std::string const& strategy
                    , std::vector<fhg::rif::entry_point> const& entry_points
                    , std::vector<fhg::rif::entry_point>& failed_entry_points
                    );
    }
  }
}
