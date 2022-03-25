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

#include <iml/rif/strategy/local.hpp>

#include <iml/rif/strategy/system_with_blocked_SIGCHLD.hpp>

#include <util-generic/hostname.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace fhg
{
  namespace iml
  {
    namespace rif
    {
      namespace strategy
      {
        namespace local
        {
          namespace
          {
            void do_local (::boost::format const& command)
            {
              util::system_with_blocked_SIGCHLD (str (command));
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
            if (!parameters.empty())
            {
              throw std::invalid_argument
                ("rif-strategy 'local' does not accept parameters");
            }

            if (all_hostnames.size() != 1)
            {
              throw std::invalid_argument
                ( "rif-strategy 'local' expects exactly one host, got "
                + std::to_string (all_hostnames.size())
                );
            }

            auto const localhost (fhg::util::hostname());
            auto const& hostname (all_hostnames.front());
            if (hostname != localhost)
            {
              throw std::invalid_argument
                ( "local strategy used to bootstrap on non-localhost host: "
                  "got=" + hostname + ", expected=" + localhost
                );
            }

            try
            {
              do_local ( ::boost::format
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

              return std::unordered_map<std::string, std::exception_ptr>{};
            }
            catch (...)
            {
              return {{hostname, std::current_exception()}};
            }
          }

          std::pair < std::unordered_set<std::string>
                    , std::unordered_map<std::string, std::exception_ptr>
                    > teardown
              ( std::unordered_map<std::string, EntryPoint> const& all_entry_points
              , std::vector<std::string> const& parameters
              )
          {
            if (!parameters.empty())
            {
              throw std::invalid_argument
                ("rif-strategy 'local' does not accept parameters");
            }

            if (all_entry_points.size() != 1)
            {
              throw std::invalid_argument
                ( "rif-strategy 'local' expects exactly one entry_point, got "
                + std::to_string (all_entry_points.size())
                );
            }

            auto const localhost (fhg::util::hostname());
            auto const& entry_point (*all_entry_points.begin());
            auto const& hostname (entry_point.first);
            if (hostname != localhost)
            {
              throw std::invalid_argument
                ( "local strategy used to teardown on non-localhost host: "
                  "got=" + hostname + ", expected=" + localhost
                );
            }

            try
            {
              do_local ( ::boost::format ("/bin/kill -TERM %1%")
                       % entry_point.second.pid
                       );

              return { {hostname}
                     , std::unordered_map<std::string, std::exception_ptr>{}
                     };
            }
            catch (...)
            {
              return { std::unordered_set<std::string>{}
                     , {{hostname, std::current_exception()}}
                     };
            }
          }
        }
      }
    }
  }
}
