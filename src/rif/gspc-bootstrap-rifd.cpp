
#include <rif/entry_point.hpp>

#include <fhg/util/boost/asio/ip/address.hpp>
#include <fhg/util/boost/program_options/validators/existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_file.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/nest_exceptions.hpp>
#include <fhg/util/print_exception.hpp>
#include <fhg/util/read_lines.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <fhg/util/wait_and_collect_exceptions.hpp>

#include <network/server.hpp>

#include <rpc/server.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace
{
  namespace option
  {
    constexpr const char* const hostfile {"hostfile"};
    constexpr const char* const port {"port"};
    constexpr const char* const strategy {"strategy"};
    constexpr const char* const gspc_home {"gspc-home"};
  }

  namespace strategy
  {
    void ssh ( std::vector<std::string> const& hostnames
             , boost::optional<unsigned short> const& port
             , std::string const& register_host
             , unsigned short register_port
             , boost::filesystem::path const& binary
             )
    {
      std::vector<std::future<void>> sshs;

      for (std::string const& hostname : hostnames)
      {
        std::string const command
          ( ( boost::format
                ("ssh %1% %2% %3% %4% --register-host %5% --register-port %6%")
            % "-q -x -T -n -o CheckHostIP=no -o StrictHostKeyChecking=no"
            % hostname
            % binary
            % (port ? "--port " + std::to_string (*port) : "")
            % register_host
            % register_port
            ).str()
          );

        sshs.emplace_back
          ( std::async
              ( std::launch::async
              , [hostname, command]
                {
                  if ( int ec
                     = fhg::util::system_with_blocked_SIGCHLD (command.c_str())
                     )
                  {
                    throw std::runtime_error
                      ( ( boost::format ("%1%: %2% failed: %3%")
                        % hostname
                        % command
                        % ec
                        ).str()
                      );
                  }
                }
              )
          );
      }

      fhg::util::wait_and_collect_exceptions (sshs);
    }
  }
}

int main (int argc, char** argv)
try
{
  std::unordered_map
    < std::string
    , std::function<void ( std::vector<std::string> const&
                         , boost::optional<unsigned short> const&
                         , std::string const&
                         , unsigned short
                         , boost::filesystem::path const&
                         )
                   >
    > const strategies {{"ssh", strategy::ssh}};

  boost::program_options::options_description options_description;
  options_description.add_options()
    ( option::port
    , boost::program_options::value
        <fhg::util::boost::program_options::positive_integral<unsigned short>>()
      //! \todo make optional and allow rif to chose port, requires to
      //! prepare next steps to deal with rif_port_by_hostname
      ->required()
    , "port to listen on"
    )
    ( option::hostfile
    , boost::program_options::value
        <fhg::util::boost::program_options::nonempty_file>()->required()
    , "hostfile"
    )
    ( option::strategy
    , boost::program_options::value<std::string>()->required()
    , ( "strategy: one of "
      + fhg::util::join (strategies | boost::adaptors::map_keys, ", ")
      ).c_str()
    )
    ( option::gspc_home
    , boost::program_options::value
        <fhg::util::boost::program_options::existing_directory>()->required()
    , "installation path of gpispace"
    )
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
      .options (options_description)
      .run()
    , vm
    );

  boost::program_options::notify (vm);

  std::string const strategy (vm.at (option::strategy).as<std::string>());

  if (!strategies.count (strategy))
  {
    throw std::invalid_argument
      (( boost::format ("invalid argument '%1%' for --%2%: one of %3%")
       % strategy
       % option::strategy
       % fhg::util::join (strategies | boost::adaptors::map_keys, ", ")
       ).str()
      );
  }

  boost::optional<unsigned short> const port
    ( vm.count (option::port)
    ? boost::make_optional<unsigned short>
        ( vm.at (option::port)
        . as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
        )
    : boost::none
    );

  std::vector<std::string> const hostnames
    ( fhg::util::read_lines
      ( vm.at (option::hostfile)
      . as<fhg::util::boost::program_options::nonempty_file>()
      )
    );

  std::mutex entry_points_guard;
  std::condition_variable entry_point_added;
  std::vector<fhg::rif::entry_point> entry_points;

  boost::asio::io_service io_service;

  fhg::rpc::service_dispatcher service_dispatcher
    {fhg::rpc::exception::serialization_functions()};

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
        strategies.at (strategy)
          ( hostnames
          , port
          , fhg::util::connectable_to_address_string (local_endpoint.address())
          , local_endpoint.port()
          , boost::filesystem::canonical
              ( vm.at (option::gspc_home)
              . as<fhg::util::boost::program_options::existing_directory>()
              ) / "bin" / "gspc-rifd"
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

  for (fhg::rif::entry_point const& entry_point : entry_points)
  {
    std::cout << entry_point.hostname
              << ' ' << entry_point.port
              << ' ' << entry_point.pid
              << '\n';
  }
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");

  return 1;
}
