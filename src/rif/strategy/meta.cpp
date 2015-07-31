// bernd.loerwald@itwm.fraunhofer.de

#include <rif/strategy/meta.hpp>

#include <network/connectable_to_address_string.hpp>
#include <util-generic/join.hpp>
#include <util-generic/nest_exceptions.hpp>

#include <rif/strategy/ssh.hpp>

#include <rpc/server_with_multiple_clients.hpp>

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
  namespace rif
  {
    namespace strategy
    {
      namespace
      {
        std::unordered_map
          < std::string
          , std::pair
            < std::function
              < std::unordered_map<std::string, std::exception_ptr>
                ( std::vector<std::string> const&
                , boost::optional<unsigned short> const&
                , std::string const&
                , unsigned short
                , boost::filesystem::path const&
                , std::vector<std::string> const&
                )
              >
            , std::function
              < std::pair < std::unordered_set<std::string>
                          , std::unordered_map<std::string, std::exception_ptr>
                          >
                ( std::unordered_map<std::string, fhg::rif::entry_point> const&
                , std::vector<std::string> const&
                )
              >
            >
          > const strategies {{"ssh", {ssh::bootstrap, ssh::teardown}}};

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
        auto const _ (strategies | boost::adaptors::map_keys);
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

      std::pair < std::unordered_map<std::string, fhg::rif::entry_point>
                , std::unordered_map<std::string, std::exception_ptr>
                > bootstrap
        ( std::string const& strategy
        , std::vector<std::string> const& hostnames_in
        , boost::optional<unsigned short> const& port
        , boost::filesystem::path const& gspc_home
        , std::vector<std::string> const& parameters
        )
      {
        validate_strategy (strategy);

        std::vector<std::string> const hostnames (stable_unique (hostnames_in));

        std::mutex entry_points_guard;
        std::condition_variable entry_point_added;
        std::unordered_map<std::string, fhg::rif::entry_point> entry_points;

        boost::asio::io_service io_service;

        fhg::rpc::service_dispatcher service_dispatcher
          {fhg::util::serialization::exception::serialization_functions()};

        fhg::rpc::service_handler<bootstrap_callback> const register_service
            ( service_dispatcher
            , [&entry_points, &entry_points_guard, &entry_point_added]
                ( std::string const& key
                , fhg::rif::entry_point const& entry_point
                )
              {
                {
                  std::unique_lock<std::mutex> const _ (entry_points_guard);
                  entry_points.emplace (key, entry_point);
                }
                entry_point_added.notify_one();
              }
            );

        fhg::rpc::server_with_multiple_clients rpc_server (service_dispatcher);

        boost::asio::ip::tcp::endpoint const local_endpoint
          (rpc_server.local_endpoint());

        std::unordered_map<std::string, std::exception_ptr> const failed
          ( strategies.at (strategy).first
              ( hostnames
              , port
              , fhg::network::connectable_to_address_string (local_endpoint.address())
              , local_endpoint.port()
              , gspc_home / "bin" / "gspc-rifd"
              , parameters
              )
          );

        {
          std::unique_lock<std::mutex> lock (entry_points_guard);
          entry_point_added.wait
            ( lock
            , [&entry_points, &hostnames, &failed]
              {
                return entry_points.size() + failed.size() == hostnames.size();
              }
            );
        }

        return {entry_points, failed};
      }
      std::pair < std::unordered_set<std::string>
                , std::unordered_map<std::string, std::exception_ptr>
                > teardown
        ( std::string const& strategy
        , std::unordered_map<std::string, fhg::rif::entry_point> const& entry_points
        , std::vector<std::string> const& parameters
        )
      {
        validate_strategy (strategy);

        return strategies.at (strategy).second (entry_points, parameters);
      }
    }
  }
}
