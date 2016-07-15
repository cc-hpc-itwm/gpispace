// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <rif/entry_point.hpp>
#include <rpc/function_description.hpp>

#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <unordered_map>
#include <unordered_set>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      FHG_RPC_FUNCTION_DESCRIPTION ( bootstrap_callback
                                   , void ( std::string // register_key
                                          , std::string // hostname()
                                          , entry_point
                                          )
                                   );

      std::vector<std::string> available_strategies();

      std::tuple < std::unordered_map<std::string, fhg::rif::entry_point>
                 , std::unordered_map<std::string, std::exception_ptr>
                 , std::unordered_map<std::string, std::string>
                 > bootstrap
        ( std::string const& strategy
        , std::vector<std::string> const& hostnames
        , boost::optional<unsigned short> const& port
        , boost::filesystem::path const& gspc_home
        , std::vector<std::string> const& parameters
        );
      std::pair < std::unordered_set<std::string>
                , std::unordered_map<std::string, std::exception_ptr>
                > teardown
        ( std::string const& strategy
        , std::unordered_map<std::string, fhg::rif::entry_point> const& entry_points
        , std::vector<std::string> const& parameters
        );
    }
  }
}
