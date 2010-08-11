// chat server example from boost.org
//    http://www.boost.org/doc/libs/1_43_0/doc/html/boost_asio/example/chat/chat_server.cpp

#include <fhglog/fhglog.hpp>

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

using boost::asio::ip::tcp;

class basic_session
{
public:
  virtual ~basic_session () {}
  virtual boost::asio::ip::tcp::endpoint remote_endpoint () const = 0;
  virtual boost::asio::ip::tcp::endpoint local_endpoint () const = 0;
};

typedef boost::shared_ptr<basic_session> basic_session_ptr;

class session_manager
{
public:
  void add (basic_session_ptr session)
  {
    LOG(INFO, "adding session between " << session->local_endpoint() << " and " << session->remote_endpoint());
    sessions_.insert(session);
  }

  void del (basic_session_ptr session)
  {
    LOG(INFO, "removing session between " << session->local_endpoint() << " and " << session->remote_endpoint());
    sessions_.erase(session);
  }
private:
  std::set<basic_session_ptr> sessions_;
};

class session_t : public basic_session
                , public boost::enable_shared_from_this<session_t>
{
public:
  session_t (boost::asio::io_service & io_service, session_manager & manager)
    : socket_(io_service)
    , manager_(manager)
  {}

  tcp::socket & socket ()
  {
    return socket_;
  }

  ~session_t ()
  {
  }

  boost::asio::ip::tcp::endpoint remote_endpoint () const
  {
    return socket_.remote_endpoint();
  }
  boost::asio::ip::tcp::endpoint local_endpoint () const
  {
    return socket_.local_endpoint();
  }

  void start ()
  {
    manager_.add (shared_from_this());
    read_header ();
  }

private:
  void read_header ()
  {
    DLOG(DEBUG, "trying to receive header of length " << header_length);
    boost::asio::async_read ( socket_
                            , boost::asio::buffer (inbound_header_)
                            , boost::bind ( &session_t::handle_read_header
                                          , shared_from_this()
                                          , boost::asio::placeholders::error
                                          , boost::asio::placeholders::bytes_transferred
                                          )
                            );
  }

  void handle_read_header ( const boost::system::error_code & error
                          , size_t bytes_recv
                          )
  {
    if (!error)
    {
      DLOG(DEBUG, "received " << bytes_recv << " bytes of header");
      std::istringstream is (std::string (inbound_header_, header_length));
      std::size_t inbound_data_size = 0;
      if (! (is >> std::hex >> inbound_data_size))
      {
        LOG(ERROR, "could not parse header: " << std::string(inbound_header_, header_length));
        manager_.del (shared_from_this());
        // TODO: call handler
        return;
      }

      DLOG(DEBUG, "going to receive " << inbound_data_size << " bytes");

      inbound_data_.resize (inbound_data_size);

      boost::asio::async_read ( socket_
                              , boost::asio::buffer (inbound_data_)
                              , boost::bind ( &session_t::handle_read_data
                                            , shared_from_this()
                                            , boost::asio::placeholders::error
                                            , boost::asio::placeholders::bytes_transferred
                                            )
                              );
    }
    else
    {
      LOG(WARN, "session closed: error := " << error);
      manager_.del (shared_from_this());
    }
  }

  void handle_read_data ( const boost::system::error_code & error
                        , size_t bytes_recv
                        )
  {
    if (!error)
    {
      DLOG(TRACE, "received " << bytes_recv << " bytes");
      std::string data(&inbound_data_[0], inbound_data_.size());
      DLOG(INFO, "  data := " << data);

      read_header ();
    }
    else
    {
      LOG(ERROR, "session got error during chunk receive := " << error);
      manager_.del (shared_from_this());
    }
  }

  tcp::socket socket_;

  enum { header_length = 8 };

  std::string outbound_header_;
  std::string outbound_data_;

  char inbound_header_[header_length];
  std::vector<char> inbound_data_;

  session_manager & manager_;
};

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
  session_manager manager_;
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
