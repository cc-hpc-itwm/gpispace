// Copyright (C) 2015-2017,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/rif/strategy/meta.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/join.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>

#include <gspc/rif/strategy/local.hpp>
#include <gspc/rif/strategy/pbsdsh.hpp>
#include <gspc/rif/strategy/ssh.hpp>

#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>



    namespace gspc::rif::strategy
    {
      namespace
      {
        struct Strategy
        {
          std::function
              < std::unordered_map<std::string, std::exception_ptr>
                ( std::vector<std::string> const&
                , std::optional<unsigned short> const&
                , std::string const&
                , unsigned short
                , std::filesystem::path const&
                , std::vector<std::string> const&
                , std::ostream&
                )
              > bootstrap;
          std::function
              < std::pair < std::unordered_set<std::string>
                          , std::unordered_map<std::string, std::exception_ptr>
                          >
                ( std::unordered_map<std::string, gspc::rif::entry_point> const&
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
              + gspc::util::join (available_strategies(), ", ").string()
              );
          }
        }
      }

      std::vector<std::string> available_strategies()
      {
        auto vs {std::vector<std::string>{}};
        vs.reserve (strategies.size());
        std::transform
          ( std::begin (strategies), std::end (strategies)
          , std::back_inserter (vs)
          , [] (auto const& kv)
            {
              auto const& [key, value] {kv};
              return key;
            }
          );
        return vs;
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

      std::tuple < std::unordered_map<std::string, gspc::rif::entry_point>
                 , std::unordered_map<std::string, std::exception_ptr>
                 , std::unordered_map<std::string, std::string>
                 > bootstrap
        ( std::string const& strategy
        , std::vector<std::string> const& hostnames_in
        , std::optional<unsigned short> const& port
        , std::filesystem::path const& gspc_home
        , std::vector<std::string> const& parameters
        , std::ostream& out
        )
      {
        validate_strategy (strategy);

        std::vector<std::string> const hostnames (stable_unique (hostnames_in));

        std::mutex entry_points_guard;
        std::condition_variable entry_point_added;
        std::unordered_map<std::string, gspc::rif::entry_point> entry_points;
        std::unordered_map<std::string, std::string> real_hostnames;

        gspc::rpc::service_dispatcher service_dispatcher;

        gspc::rpc::service_handler<bootstrap_callback> const register_service
            ( service_dispatcher
            , [&entry_points, &entry_points_guard, &entry_point_added
              , &real_hostnames
              ]
                ( std::string const& key
                , std::string hostname
                , gspc::rif::entry_point const& entry_point
                )
              {
                {
                  std::lock_guard<std::mutex> const _ (entry_points_guard);
                  entry_points.emplace (key, entry_point);
                  real_hostnames.emplace (key, hostname);
                }
                entry_point_added.notify_one();
              }
            , gspc::rpc::not_yielding
            );

        gspc::util::scoped_boost_asio_io_service_with_threads io_service (2);
        gspc::rpc::service_tcp_provider rpc_server
          (io_service, service_dispatcher);

        ::boost::asio::ip::tcp::endpoint const local_endpoint
          (rpc_server.local_endpoint());

        std::unordered_map<std::string, std::exception_ptr> const failed
          ( strategies.at (strategy).bootstrap
              ( hostnames
              , port
              , gspc::util::connectable_to_address_string (local_endpoint.address())
              , local_endpoint.port()
              , gspc_home / "bin" / "gspc-rifd"
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
        , std::unordered_map<std::string, gspc::rif::entry_point> const& entry_points
        , std::vector<std::string> const& parameters
        )
      {
        validate_strategy (strategy);

        return strategies.at (strategy).teardown (entry_points, parameters);
      }
    }
