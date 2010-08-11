// chat server example from boost.org
//    http://www.boost.org/doc/libs/1_43_0/doc/html/boost_asio/example/chat/chat_server.cpp

#include <fhglog/fhglog.hpp>

#include <ios>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <fhgcom/session_manager.hpp>
#include <fhgcom/session.hpp>

using boost::asio::ip::tcp;

typedef boost::shared_ptr<fhg::com::basic_session> basic_session_ptr;
typedef fhg::com::session session_t;
typedef boost::shared_ptr<session_t> session_ptr;

class tcp_server
{
public:
  tcp_server ( boost::asio::io_service & io_service
             , const std::string & host
             , const std::string & service
             , const bool reuse_addr = true
             )
    : io_service_ (io_service)
    , acceptor_(io_service)
  {
    boost::asio::ip::tcp::resolver resolver (io_service);
    boost::asio::ip::tcp::resolver::query query(host, service);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator
      (resolver.resolve (query));

    boost::system::error_code ec;

    while (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      boost::asio::ip::tcp::endpoint endpoint (*endpoint_iterator);
      DLOG(TRACE, "trying to bind to " << endpoint);
      acceptor_.open (endpoint.protocol(), ec);
      boost::asio::detail::throw_error (ec);

      if (reuse_addr)
      {
        acceptor_.set_option (tcp::acceptor::reuse_address(true), ec);
        boost::asio::detail::throw_error (ec);
      }

      acceptor_.bind (endpoint, ec);
      if (! ec)
      {
        acceptor_.listen (tcp::acceptor::max_connections, ec);
        if (! ec)
        {
          LOG(INFO, "successfully bound to: " << acceptor_.local_endpoint());
          accept ();
          break;
        }
      }

      LOG(WARN, "could not bind to " << endpoint << " = " << ec.message() << ": " << ec);

      acceptor_.close();
      ++endpoint_iterator;
    }

    // throw remaining error if any
    boost::asio::detail::throw_error (ec);
  }

private:
  void accept ()
  {
    session_ptr new_session(new session_t(io_service_, manager_));
    acceptor_.async_accept
      ( new_session->socket()
      , boost::bind ( &tcp_server::handle_accept
                    , this
                    , new_session
                    , boost::asio::placeholders::error
                    )
      );
  }

  void handle_accept ( session_ptr session
                     , const boost::system::error_code & error
                     )
  {
    if (!error)
    {
      session->start();
      accept();
    }
  }

  boost::asio::io_service & io_service_;
  tcp::acceptor acceptor_;
  fhg::com::session_manager manager_;
};

int main(int ac, char *av[])
{
  namespace po = boost::program_options;

  FHGLOG_SETUP(ac,av);

  std::string server_address ("");
  std::string server_port ("1234");
  bool reuse_address (true);

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("bind,b", po::value<std::string>(&server_address)->default_value(server_address), "bind to this address")
    ("port,p", po::value<std::string>(&server_port)->default_value(server_port), "port or service name to use")
    ("reuse-address", po::value<bool>(&reuse_address)->default_value(reuse_address), "reuse address")
    ;

  po::variables_map vm;
  try
  {
    po::store (po::parse_command_line (ac, av, desc), vm);
    po::notify (vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "try " << av[0] << " -h to get some help" << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count ("help"))
  {
    std::cerr << "usage: " << av[0] << std::endl;
    std::cerr << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  boost::asio::io_service io_service;
  tcp_server server (io_service, server_address, server_port, reuse_address);
  io_service.run();
  return 0;
}
