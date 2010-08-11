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

using boost::asio::ip::tcp;

class tcp_client : public boost::enable_shared_from_this<tcp_client>
{
public:
  tcp_client ( boost::asio::io_service & io_service
             , const std::string & host
             , const std::string & service
             )
    : socket_(io_service)
    , send_in_progress_(false)
  {
    boost::asio::ip::tcp::resolver resolver (io_service);
    boost::asio::ip::tcp::resolver::query query(host, service);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator
      (resolver.resolve (query));
    boost::asio::ip::tcp::endpoint endpoint (*endpoint_iterator);

    DLOG (TRACE, "trying to connect to " << endpoint);
    socket_.async_connect( endpoint
                         , boost::bind( &tcp_client::handle_connect
                                      , this
                                      , boost::asio::placeholders::error
                                      , ++endpoint_iterator
                                      )
                         );
  }

  void close ()
  {
    while (to_send_.size())
      sleep(1);
    socket_.shutdown(tcp::socket::shutdown_both);
    socket_.close();
  }

  void send ( const std::string & data )
  {
    std::ostringstream sstr;
    sstr << std::setw(header_length)
         << std::setfill('0')
         << std::hex
         << data.size()
         << data
      ;

    bool send_in_progress_ = !to_send_.empty();
    to_send_.push_back (sstr.str());
    if (!send_in_progress_)
    {
      DLOG(TRACE, "initiating write of " << to_send_.front());

      boost::asio::async_write( socket_
                              , boost::asio::buffer( to_send_.front().data()
                                                   , to_send_.front().length()
                                                   )
                              , boost::bind( &tcp_client::handle_write
                                           , this
                                           , boost::asio::placeholders::error
                                           )
                              );
    }
  }

private:
  void handle_connect ( const boost::system::error_code & e
                      , boost::asio::ip::tcp::resolver::iterator endpoint_iterator
                      )
  {
    if (! e)
    {
      DLOG (TRACE, "connected");
      read_header();
    }
    else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      socket_.close();
      boost::asio::ip::tcp::endpoint endpoint (*endpoint_iterator);

      DLOG (TRACE, "trying to connect to " << endpoint);
      socket_.async_connect( endpoint
                           , boost::bind( &tcp_client::handle_connect
                                        , this
                                        , boost::asio::placeholders::error
                                        , ++endpoint_iterator
                                        )
                           );
    }
    else
    {
      LOG(ERROR, "could not connect: " << e.message() << ": " << e);
    }
  }

  void handle_write (const boost::system::error_code & e)
  {
    if (! e)
    {
      DLOG(TRACE, "write completed");

      to_send_.pop_front();
      if (! to_send_.empty())
      {
        boost::asio::async_write( socket_
                                , boost::asio::buffer( to_send_.front().data()
                                                     , to_send_.front().length()
                                                     )
                                , boost::bind( &tcp_client::handle_write
                                             , this
                                             , boost::asio::placeholders::error
                                             )
                                );
      }
    }
    else
    {
      LOG(ERROR, "could not send data: " << e << ": " << e.message());
    }
  }

  void read_header ()
  {
    DLOG(DEBUG, "trying to receive header of length " << header_length);
    boost::asio::async_read ( socket_
                            , boost::asio::buffer (inbound_header_)
                            , boost::bind ( &tcp_client::handle_read_header
                                          , this
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
        // TODO: call handler
        return;
      }

      DLOG(DEBUG, "going to receive " << inbound_data_size << " bytes");

      inbound_data_.resize (inbound_data_size);

      boost::asio::async_read ( socket_
                              , boost::asio::buffer (inbound_data_)
                              , boost::bind ( &tcp_client::handle_read_data
                                            , this
                                            , boost::asio::placeholders::error
                                            , boost::asio::placeholders::bytes_transferred
                                            )
                              );
    }
    else
    {
      LOG(WARN, "session closed: " << error.message() << " := " << error);
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
    }
  }

  enum { header_length = 8 };
  char inbound_header_[header_length];
  std::vector<char> inbound_data_;

  tcp::socket socket_;
  bool send_in_progress_;
  std::list<std::string> to_send_;
};

typedef boost::shared_ptr<tcp_client> client_ptr;

struct service_thread
{
  service_thread ()
  {}

  boost::asio::io_service & service () { return io_service_; }

  void operator () ()
  {
    DLOG(TRACE, "thread started");
    io_service_.run();
    io_service_.reset();
    DLOG(TRACE, "thread stopped");
  }

  boost::asio::io_service io_service_;
};

int main(int ac, char *av[])
{
  FHGLOG_SETUP(ac,av);

  std::string server_address ("localhost");
  std::string server_port ("1234");

  if (ac > 1)
  {
    server_address = av[1];
  }

  if (ac > 2)
  {
    server_port = av[2];
  }

  // fork service thread
  service_thread thrd;
  client_ptr client (new tcp_client(thrd.service(), server_address, server_port));

  boost::thread io_service_thread = boost::thread( boost::ref(thrd) );

  while ( std::cin )
  {
    std::cerr << "reading message..." << std::endl;
    std::string input;
    std::getline (std::cin, input);
    if (input.empty())
      continue;
    else if (input == "quit")
      break;
    std::cerr << "sending " << input << std::endl;
    client->send (input);
  }

  client->close ();

  thrd.service().stop();
  io_service_thread.interrupt();
  io_service_thread.join();

  return 0;
}
