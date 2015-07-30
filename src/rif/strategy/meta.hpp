// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <rif/entry_point.hpp>
#include <rpc/function_description.hpp>

#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <unordered_map>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      FHG_RPC_FUNCTION_DESCRIPTION ( bootstrap_callback
                                   , void (std::string, entry_point)
                                   );

      std::vector<std::string> available_strategies();

      std::unordered_map<std::string, fhg::rif::entry_point> bootstrap
        ( std::string const& strategy
        , std::vector<std::string> const& hostnames
        , boost::optional<unsigned short> const& port
        , boost::filesystem::path const& gspc_home
        );
      void teardown
        ( std::string const& strategy
        , std::unordered_map<std::string, fhg::rif::entry_point> const& entry_points
        , std::unordered_map<std::string, fhg::rif::entry_point>& failed_entry_points
        );
    }
  }
}
