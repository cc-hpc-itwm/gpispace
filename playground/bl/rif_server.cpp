// bernd.loerwald@itwm.fraunhofer.de

#include <playground/bl/rpc/server.hpp>

#include <fhg/syscall.hpp>

#include <playground/bl/net/server.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <thread>
#include <vector>

struct rif
{
public:
  rif (unsigned short port, boost::asio::io_service& io_service)
    : _service_dispatcher()
    , _start_service ( _service_dispatcher
                     , "start"
                     , thunk<pid_t (std::string, std::vector<std::string>)>
                       (std::bind (&rif::start, this, std::placeholders::_1, std::placeholders::_2))
                     )
    , _stop_service ( _service_dispatcher
                    , "stop"
                    , thunk<void (std::size_t)>
                      (std::bind (&rif::stop, this, std::placeholders::_1))
                    )
    , _acceptor ( boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4(), port)
                , io_service
                , [] (buffer_type buf) { return buf; }
                , [] (buffer_type buf) { return buf; }
                , [this] (connection_type* connection, buffer_type message)
                {
                  _service_dispatcher.dispatch (connection, message);
                }
                , [this] (connection_type* connection)
                {
                  _connections.erase
                    ( std::find_if
                      ( _connections.begin()
                      , _connections.end()
                      , [&connection]
                        (std::unique_ptr<connection_type> const& other)
                      {
                        return other.get() == connection;
                      }
                      )
                    );
                }
                , [this] (std::unique_ptr<connection_type> connection)
                {
                  _connections.push_back (std::move (connection));
                }
                )
  {}

private:
  pid_t start (std::string filename, std::vector<std::string> arguments)
  {
    std::this_thread::sleep_for (std::chrono::seconds (1));
    pid_t pid (fhg::syscall::fork());
    if (pid)
    {
      return pid;
    }
    else
    {
      std::vector<char> argv_buffer;
      std::vector<char*> argv;

      {
        arguments.insert (arguments.begin(), filename);
        std::vector<std::size_t> argv_offsets;
        for (std::string arg : arguments)
        {
          std::size_t pos (argv_buffer.size());
          argv_buffer.resize (argv_buffer.size() + arg.size() + 1);
          std::copy (arg.begin(), arg.end(), argv_buffer.data() + pos);
          argv_buffer[argv_buffer.size() - 1] = '\0';
          argv_offsets.push_back (pos);
        }
        for (std::size_t offset : argv_offsets)
        {
          argv.push_back (argv_buffer.data() + offset);
        }
        argv.push_back (nullptr);
      }

      try
      {
        fhg::syscall::execve (argv[0], argv.data(), nullptr);
      }
      catch (boost::system::system_error const&)
      {
        _exit (127);
      }
    }
  }
  void stop (pid_t pid)
  {
    fhg::syscall::kill (pid, 9);
  }

  service_dispatcher _service_dispatcher;
  service_handler _start_service;
  service_handler _stop_service;

  std::vector<std::unique_ptr<connection_type>> _connections;

  continous_acceptor<boost::asio::ip::tcp> _acceptor;
};


int main (int argc, char** argv)
{
  boost::asio::io_service io_service;

  const rif _ (13331, io_service);
  const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    io_service_thread ([&io_service]() { io_service.run(); });

  return 0;
}
