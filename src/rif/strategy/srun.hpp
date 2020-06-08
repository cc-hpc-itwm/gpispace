#pragma once

#include <rif/entry_point.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <exception>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      namespace srun
      {
        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& hostnames
                    , boost::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , boost::filesystem::path const& binary
                    , std::vector<std::string> const& parameters
                    , std::ostream&
                    );
        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
          ( std::unordered_map<std::string, fhg::rif::entry_point> const&
          , std::vector<std::string> const& parameters
          );
      }
    }
  }
}
