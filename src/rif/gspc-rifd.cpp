// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/syscall.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/boost/serialization/path.hpp>
#include <fhg/util/boost/serialization/unordered_map.hpp>
#include <fhg/util/print_exception.hpp>

#include <network/server.hpp>

#include <rif/execute_and_get_startup_messages.hpp>

#include <rpc/server.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/scoped_thread.hpp>

namespace
{
  namespace option
  {
    constexpr const char* const port {"port"};
  }

  void reap_children (int)
  {
    try
    {
      while (fhg::syscall::waitpid (-1, nullptr, WNOHANG) > 0)
      {
        //! \note Just wait for all pending ones to reap them.
        //! see http://www.microhowto.info/howto/reap_zombie_processes_using_a_sigchld_handler.html
      }
    }
    catch (boost::system::system_error const& ex)
    {
      if (ex.code() == boost::system::errc::no_child_process)
      {
        return;
      }

      throw;
    }
  }
}

int main (int argc, char** argv)
try
{
  boost::program_options::options_description options_description;
  options_description.add_options()
    ( option::port
    , boost::program_options::value
        <fhg::util::boost::program_options::positive_integral<unsigned short>>()
        ->required()
    , "port to listen on"
    );

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
      .options (options_description)
      .run()
    , vm
    );

  boost::program_options::notify (vm);

  unsigned short const port
    ( vm.at (option::port)
      .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
    );

  boost::asio::io_service io_service;

  fhg::rpc::service_dispatcher service_dispatcher
    {fhg::rpc::exception::serialization_functions()};

  fhg::rpc::service_handler<decltype (fhg::rif::execute_and_get_startup_messages)>
    execute_and_get_startup_messages_service
      ( service_dispatcher
      , "execute_and_get_startup_messages"
      , &fhg::rif::execute_and_get_startup_messages
      );

  fhg::rpc::service_handler<void (std::vector<pid_t>)> kill_service
    ( service_dispatcher
    , "kill"
    , [] (std::vector<pid_t> const& pids)
      {
        for (pid_t pid : pids)
        {
          fhg::syscall::kill (pid, SIGTERM);
        }
      }
    );

  std::vector<std::unique_ptr<fhg::network::connection_type>> connections;

  fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor
    ( boost::asio::ip::tcp::endpoint (boost::asio::ip::address(), port)
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

  io_service.notify_fork (boost::asio::io_service::fork_prepare);
  if (pid_t child_child = fhg::syscall::fork())
  {
    io_service.notify_fork (boost::asio::io_service::fork_parent);

    std::cout << child_child << std::endl;

    return 0;
  }

  fhg::syscall::close (0);
  fhg::syscall::close (1);
  fhg::syscall::close (2);

  io_service.notify_fork (boost::asio::io_service::fork_child);

  struct scoped_child_reaper
  {
    scoped_child_reaper()
    {
      struct sigaction sigact;
      memset (&sigact, 0, sizeof (sigact));

      sigact.sa_handler = reap_children;
      sigact.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;

      fhg::syscall::sigaction (SIGCHLD, &sigact, &_orig);
    }
    ~scoped_child_reaper()
    {
      fhg::syscall::sigaction (SIGCHLD, &_orig, nullptr);
    }

    struct sigaction _orig;
  } const scoped_child_reaper;

  const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    io_service_thread ([&io_service]() { io_service.run(); });

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");
}
