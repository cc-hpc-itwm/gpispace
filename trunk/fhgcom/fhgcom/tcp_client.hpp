#ifndef FHG_COM_TCP_CLIENT_HPP
#define FHG_COM_TCP_CLIENT_HPP 1

#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/asio.hpp>

namespace fhg
{
  namespace com
  {
    class tcp_client : public boost::enable_shared_from_this<tcp_client>
    {
    public:
      typedef boost::unique_lock<boost::recursive_mutex> lock_t;

      tcp_client ( boost::asio::io_service & io_service
                 , const std::string & host
                 , const std::string & port
                 );

      virtual ~tcp_client ();

      void start ();
      void start (const std::string & host, const std::string & port);
      void start (boost::asio::ip::tcp::resolver::iterator endpoint_iter);

      void stop ();
      void send ( const std::string & data );
      std::string recv ();
    private:
      void start_connect ( boost::asio::ip::tcp::resolver::iterator );

      void handle_connect ( const boost::system::error_code &
                          , boost::asio::ip::tcp::resolver::iterator
                          );

      void handle_write (const boost::system::error_code &);

      void start_writer ();
      void start_reader ();

      void read_header ();

      void handle_read_header ( const boost::system::error_code &
                              , size_t
                              );

      void handle_read_data ( const boost::system::error_code &
                            , size_t
                            );

      void check_deadline ();

      mutable boost::recursive_mutex mutex_;
      boost::condition data_sent_;
      boost::condition data_rcvd_;

      enum { header_length = 8 };
      char inbound_header_[header_length];
      std::vector<char> inbound_data_;

      boost::asio::io_service & io_service_;
      boost::asio::ip::tcp::socket socket_;
      std::string host_;
      std::string port_;
      std::list<std::string> to_send_;
      std::list<std::string> to_recv_;
      bool send_in_progress_;
      bool stopped_;
      bool connected_;

      boost::asio::deadline_timer deadline_;
    };
  }
}

#endif
