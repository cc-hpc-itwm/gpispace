// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <iml/rif/strategy/meta.hpp>

#include <iml/detail/Installation.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/join.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <iml/rif/strategy/local.hpp>
#include <iml/rif/strategy/pbsdsh.hpp>
#include <iml/rif/strategy/ssh.hpp>

#include <util-rpc/service_tcp_provider.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace fhg
{
  namespace iml
  {
    namespace rif
    {
      namespace strategy
      {
        namespace
        {
          struct Strategy
          {
            std::function
              < std::unordered_map<std::string, std::exception_ptr>
                  ( std::vector<std::string> const&
                  , ::boost::optional<unsigned short> const&
                  , std::string const&
                  , unsigned short
                  , ::boost::filesystem::path const&
                  , std::vector<std::string> const&
                  , std::ostream&
                  )
              > bootstrap;
            std::function
              < std::pair < std::unordered_set<std::string>
                          , std::unordered_map<std::string, std::exception_ptr>
                          >
                  ( std::unordered_map<std::string, EntryPoint> const&
                  , std::vector<std::string> const&
                  )
              > teardown;
          };

          std::unordered_map <std::string, Strategy> const strategies
            { {"local", {local::bootstrap, local::teardown}}
            , {"pbsdsh", {pbsdsh::bootstrap, pbsdsh::teardown}}
            , {"ssh", {ssh::bootstrap, ssh::teardown}}
            };

          void validate_strategy (std::string const& strategy)
          {
            if (!strategies.count (strategy))
            {
              throw std::invalid_argument
                ("invalid strategy '" + strategy + "'. available strategies: "
                + fhg::util::join (available_strategies(), ", ").string()
                );
            }
          }
        }

        std::vector<std::string> available_strategies()
        {
          auto const _ (strategies | ::boost::adaptors::map_keys);
          return {_.begin(), _.end()};
        }

        namespace
        {
          std::vector<std::string> stable_unique (std::vector<std::string> in)
          {
            std::vector<std::string> res;
            std::unordered_set<std::string> seen;
            for (std::string& elem : in)
            {
              if (seen.emplace (elem).second)
              {
                res.emplace_back (std::move (elem));
              }
            }
            return res;
          }
        }

        std::tuple < std::unordered_map<std::string, EntryPoint>
                   , std::unordered_map<std::string, std::exception_ptr>
                   , std::unordered_map<std::string, std::string>
                   > bootstrap
          ( std::string const& strategy
          , std::vector<std::string> const& hostnames_in
          , ::boost::optional<unsigned short> const& port
          , std::vector<std::string> const& parameters
          , std::ostream& out
          )
        {
          validate_strategy (strategy);

          std::vector<std::string> const hostnames (stable_unique (hostnames_in));

          std::mutex entry_points_guard;
          std::condition_variable entry_point_added;
          std::unordered_map<std::string, EntryPoint> entry_points;
          std::unordered_map<std::string, std::string> real_hostnames;

          fhg::rpc::service_dispatcher service_dispatcher;

          fhg::rpc::service_handler<bootstrap_callback> const register_service
            ( service_dispatcher
            , [&entry_points, &entry_points_guard, &entry_point_added
              , &real_hostnames
              ]
                ( std::string const& key
                , std::string hostname
                , EntryPoint const& entry_point
                )
              {
                {
                  std::lock_guard<std::mutex> const _ (entry_points_guard);
                  entry_points.emplace (key, entry_point);
                  real_hostnames.emplace (key, hostname);
                }
                entry_point_added.notify_one();
              }
            , fhg::rpc::not_yielding
            );

          util::scoped_boost_asio_io_service_with_threads io_service (2);
          fhg::rpc::service_tcp_provider rpc_server
            (io_service, service_dispatcher);

          ::boost::asio::ip::tcp::endpoint const local_endpoint
            (rpc_server.local_endpoint());

          ::iml::detail::Installation const installation;

          std::unordered_map<std::string, std::exception_ptr> const failed
            ( strategies.at (strategy).bootstrap
                ( hostnames
                , port
                , fhg::util::connectable_to_address_string (local_endpoint.address())
                , local_endpoint.port()
                , installation.rifd_binary
                , parameters
                , out
                )
            );

          {
            std::unique_lock<std::mutex> lock (entry_points_guard);
            entry_point_added.wait
              ( lock
              , [&entry_points, &hostnames, &failed]
                {
                  //! not just do once: new entry_points may be added
                  //! later on, thus re-remove every time they changed.
                  for (auto&& failure : failed)
                  {
                    entry_points.erase (failure.first);
                  }

                  return entry_points.size() + failed.size() == hostnames.size();
                }
              );
          }

          return std::make_tuple (entry_points, failed, real_hostnames);
        }
        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
          ( std::string const& strategy
          , std::unordered_map<std::string, EntryPoint> const& entry_points
          , std::vector<std::string> const& parameters
          )
        {
          validate_strategy (strategy);

          return strategies.at (strategy).teardown (entry_points, parameters);
        }
      }
    }
  }
}
