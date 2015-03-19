// bernd.loerwald@itwm.fraunhofer.de

#include <rif/strategy/meta.hpp>

#include <fhg/util/boost/asio/ip/address.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/nest_exceptions.hpp>

#include <network/server.hpp>

#include <rif/strategy/ssh.hpp>

#include <rpc/server.hpp>

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
          , std::pair < std::function
                          < void ( std::vector<std::string> const&
                                 , boost::optional<unsigned short> const&
                                 , std::string const&
                                 , unsigned short
                                 , boost::filesystem::path const&
                                 )
                          >
                      , std::function
                          < void ( std::vector<fhg::rif::entry_point> const&
                                 , std::vector<fhg::rif::entry_point>&
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
              + fhg::util::join (available_strategies(), ", ")
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

      std::vector<fhg::rif::entry_point> bootstrap
        ( std::string const& strategy
        , std::vector<std::string> const& hostnames_in
        , boost::optional<unsigned short> const& port
        , boost::filesystem::path const& gspc_home
        )
      {
        validate_strategy (strategy);

        std::vector<std::string> const hostnames (stable_unique (hostnames_in));

        std::mutex entry_points_guard;
        std::condition_variable entry_point_added;
        std::vector<fhg::rif::entry_point> entry_points;

        boost::asio::io_service io_service;

        fhg::rpc::service_dispatcher service_dispatcher
          {fhg::util::serialization::exception::serialization_functions()};

        fhg::rpc::service_handler<void (fhg::rif::entry_point)>
          const register_service
            ( service_dispatcher
            , "register"
            , [&entry_points, &entry_points_guard, &entry_point_added]
                (fhg::rif::entry_point const& entry_point)
              {
                {
                  std::unique_lock<std::mutex> const _ (entry_points_guard);
                  entry_points.emplace_back (entry_point);
                }
                entry_point_added.notify_one();
              }
            );

        std::vector<std::unique_ptr<fhg::network::connection_type>> connections;

        fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor
          ( boost::asio::ip::tcp::endpoint()
          , io_service
          , [] (fhg::network::buffer_type buf) { return buf; }
          , [] (fhg::network::buffer_type buf) { return buf; }
          , [&service_dispatcher]
              (fhg::network::connection_type* connection, fhg::network::buffer_type message)
          {
            service_dispatcher.dispatch (connection, message);
          }
          , [&connections] (fhg::network::connection_type* connection)
          {
            connections.erase
              ( std::find_if
                ( connections.begin()
                , connections.end()
                , [&connection]
                    (std::unique_ptr<fhg::network::connection_type> const& other)
                  {
                    return other.get() == connection;
                  }
                )
              );
          }
          , [&connections] (std::unique_ptr<fhg::network::connection_type> connection)
            {
              connections.emplace_back (std::move (connection));
            }
          );

        const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
          io_service_thread ([&io_service]() { io_service.run(); });

        struct stop_io_service_on_scope_exit
        {
          ~stop_io_service_on_scope_exit()
          {
            _io_service.stop();
          }
          boost::asio::io_service& _io_service;
        } stop_io_service_on_scope_exit {io_service};

        boost::asio::ip::tcp::endpoint const local_endpoint
          (acceptor.local_endpoint());

        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              strategies.at (strategy).first
                ( hostnames
                , port
                , fhg::util::connectable_to_address_string (local_endpoint.address())
                , local_endpoint.port()
                , gspc_home / "bin" / "gspc-rifd"
                );
            }
          , "bootstrap-" + strategy + " failed"
          );

        {
          std::unique_lock<std::mutex> lock (entry_points_guard);
          entry_point_added.wait
            ( lock
            , [&entry_points, &hostnames]
              {
                return entry_points.size() == hostnames.size();
              }
            );
        }

        return entry_points;
      }
      void teardown ( std::string const& strategy
                    , std::vector<fhg::rif::entry_point> const& entry_points
                    , std::vector<fhg::rif::entry_point>& failed_entry_points
                    )
      {
        validate_strategy (strategy);

        fhg::util::nest_exceptions<std::runtime_error>
          ( [&]
            {
              strategies.at (strategy).second (entry_points, failed_entry_points);
            }
          , "teardown-" + strategy + " failed: "
          );
      }
    }
  }
}
