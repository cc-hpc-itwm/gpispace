// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <rif/strategy/pbsdsh.hpp>

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <util-generic/blocked.hpp>
#include <util-generic/boost/program_options/generic.hpp>
#include <util-generic/boost/program_options/validators/positive_integral.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      namespace pbsdsh
      {
        namespace
        {
          namespace option
          {
            namespace po = fhg::util::boost::program_options;

            po::option<std::size_t, po::positive_integral<std::size_t>>
              const block_size { "pbsdshs-at-once"
                               , "how many PBSDSH connections to establish at once"
                               , 64
                               };
          }

#define EXTRACT_PARAMETERS(parameters_)                                 \
          auto const vm ( option::po::options ("pbsdsh")                \
                        . add (option::block_size)                      \
                        . store_and_notify (parameters_)                \
                        );                                              \
                                                                        \
          auto const block_size (option::block_size.get_from (vm))

          void do_pbsdsh
            (std::string const& hostname, ::boost::format const& command)
          {
            //! \todo use torque's tm_spawn API directly to avoid system()
            util::system_with_blocked_SIGCHLD
              (str (::boost::format ("pbsdsh -h %1% %2%") % hostname % command));
          }
        }

        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& all_hostnames
                    , ::boost::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , ::boost::filesystem::path const& binary
                    , std::vector<std::string> const& parameters
                    , std::ostream&
                    )
        {
          EXTRACT_PARAMETERS (parameters);

          return util::blocked_async<std::string>
            ( all_hostnames
            , block_size
            , [] (std::string const& hostname) { return hostname; }
            , [&] (std::string const& hostname)
              {
                do_pbsdsh ( hostname
                          , ::boost::format
                              ( "%1%"
                                "%2%"
                                " --register-host %3% --register-port %4%"
                                " --register-key %5%"
                              )
                          % binary
                          % (port ? " --port " + std::to_string (*port) : "")
                          % register_host
                          % register_port
                          % hostname
                          );
              }
            ).second;
        }

        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
            ( std::unordered_map<std::string, fhg::rif::entry_point> const& all_entry_points
            , std::vector<std::string> const& parameters
            )
        {
          EXTRACT_PARAMETERS (parameters);

          return util::blocked_async<std::string>
            ( all_entry_points
            , block_size
            , [] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                return entry_point.first;
              }
            , [&] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                do_pbsdsh ( entry_point.second.hostname
                          , ::boost::format ("/bin/kill -TERM %1%")
                          % entry_point.second.pid
                          );
              }
            );
        }
      }
    }
  }
}
